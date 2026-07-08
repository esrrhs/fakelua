// ─────────────────────────────────────────────────────────────────────────
// pipeline 测试
//
// 测试策略：走标准管线 CompileFileTo，验证各阶段的中间结果。
// Lua 测试脚本放在 test/lua/pipeline/ 目录中。
// ─────────────────────────────────────────────────────────────────────────

#include "compile/compile_common.h"
#include "compile/compiler.h"
#include "compile/infer/cfg.h"
#include "compile/infer/shape_type.h"
#include "compile/infer/ssa.h"
#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

// 注意: CI 中测试运行目录是 build/bin/, lua 文件在 lua/ 子目录中
// 因此路径前缀是 ./lua/pipeline/

// ─────────────────────────────────────────────────────────────────────────
// 前向定义的辅助函数
// ─────────────────────────────────────────────────────────────────────────

// 编译 pipeline 测试目录中的 Lua 文件并返回完整管线结果
// lua_file 为相对路径, 格式如 "./pipeline/test_cfg_sequential.lua"
// (运行时目录为 build/bin/, Lua 文件由 POST_BUILD copy 到 build/bin/lua/)
//
// 注意: 需要创建 State 实例，因为 CompileFileTo 内部要通过 State 访问
// 编译器和 JIT 环境。即使禁用了 JIT，State 仍然是必需的。
static CompileResult RunPipeline(const std::string &lua_file) {
    CompileConfig cfg;
    cfg.debug_mode = false;
    cfg.record_c_code = true;
    cfg.disable_jit[JIT_TCC] = true;
    cfg.disable_jit[JIT_GCC] = true;
    // 使用 RAII 守卫管理 State 生命周期
    FakeluaStateGuard guard;
    return CompileFile(guard.GetState(), lua_file, cfg);
}

// 基本顺序语句: 顶层代码中 entry 块就是函数体, 所以是 entry + exit = 2 blocks
TEST(pipeline, cfg_sequential) {
    auto result = RunPipeline("./pipeline/test_cfg_sequential.lua");
    auto cfg_ptr = result.GetInferResult().cfg_functions.at("__fakelua_init");

    ASSERT_GE(cfg_ptr->blocks.size(), 2u);// entry + exit
    ASSERT_EQ(cfg_ptr->entry_id, 0);
}

// if-else: 应有 merge 块 (2 个前驱)
TEST(pipeline, cfg_if_else_merge) {
    auto result = RunPipeline("./pipeline/test_cfg_if.lua");
    auto cfg_ptr = result.GetInferResult().cfg_functions.at("test");

    bool found_merge = false;
    for (const auto &blk: cfg_ptr->blocks) {
        if (blk.pred_ids.size() == 2) {
            found_merge = true;
            break;
        }
    }
    EXPECT_TRUE(found_merge);
}

// ─────────────────────────────────────────────────────────────────────────
// SSA 构造测试 (通过底层 builder 验证)
// ─────────────────────────────────────────────────────────────────────────

// if-else 合并处插入 φ 节点
TEST(pipeline, ssa_phi_insertion) {
    auto result = RunPipeline("./pipeline/test_ssa_phi.lua");
    auto ssa_ptr = result.GetInferResult().ssa_functions.at("test");

    bool found_phi = false;
    for (const auto &[bid, phis]: ssa_ptr->block_phis) {
        if (!phis.empty()) {
            found_phi = true;
            break;
        }
    }
    EXPECT_TRUE(found_phi) << "if-else merge should insert φ nodes";
}

// 顶层 chunk 不应插入 φ (单前驱)
TEST(pipeline, ssa_main_no_phi) {
    auto result = RunPipeline("./pipeline/test_ssa_phi.lua");
    auto cfg_ptr = result.GetInferResult().cfg_functions.at("test");
    auto ssa_ptr = result.GetInferResult().ssa_functions.at("test");

    // 顶层入口块 (entry_id) 不应有 φ (从 entry 进入, 只有一个前驱)
    auto it = ssa_ptr->block_phis.find(cfg_ptr->entry_id);
    EXPECT_TRUE(it == ssa_ptr->block_phis.end() || it->second.empty());
}

// ─────────────────────────────────────────────────────────────────────────
// 类型推导测试
// ─────────────────────────────────────────────────────────────────────────

// 字面量类型推导
TEST(pipeline, type_literals) {
    auto result = RunPipeline("./pipeline/test_type_literal.lua");

    // 查找各种字面量类型
    bool found_int = false, found_string = false;
    for (const auto &[node, ty]: result.GetNodeTypes()) {
        if (ty.type == T_INT) found_int = true;
        if (ty.type == T_STRING) found_string = true;
    }
    EXPECT_TRUE(found_int);
    EXPECT_TRUE(found_string);
}

// 二元运算类型推导
TEST(pipeline, type_binop) {
    auto result = RunPipeline("./pipeline/test_type_binop.lua");

    // a + b 结果应该是 T_INT
    bool found_binop = false;
    for (const auto &[node, ty]: result.GetNodeTypes()) {
        if (ty.type == T_INT && node->Type() == SyntaxTreeType::Exp) {
            found_binop = true;
        }
    }
    EXPECT_TRUE(found_binop);
}

// ─────────────────────────────────────────────────────────────────────────
// Shape 测试
// ─────────────────────────────────────────────────────────────────────────

// table 字面量构造 shape
TEST(pipeline, shape_literal) {
    auto result = RunPipeline("./pipeline/test_shape_literal.lua");

    // shape_registry 应该有 shape
    ASSERT_TRUE(result.GetShapeRegistry() != nullptr);
    EXPECT_GE(result.GetShapeRegistry()->Count(), 1);

    // 查找包含字段 b 和 c 的 shape
    bool found_shape = false;
    for (int sid = 0; sid < result.GetShapeRegistry()->Count(); ++sid) {
        const ShapeType &shape = result.GetShapeRegistry()->Get(sid);
        if (shape.fields.size() == 2) {
            bool has_b = false, has_c = false;
            for (const auto &f: shape.fields) {
                if (f.name == "b" && f.type == T_INT) has_b = true;
                if (f.name == "c" && f.type == T_FLOAT) has_c = true;
            }
            if (has_b && has_c) {
                found_shape = true;
                break;
            }
        }
    }
    EXPECT_TRUE(found_shape) << "should have shape {b: int, c: float}";
}

// 字段访问命中 shape
TEST(pipeline, shape_field_access) {
    auto result = RunPipeline("./pipeline/test_shape_field_access.lua");

    // a.x 节点类型应为 T_INT
    bool found_field_access = false;
    for (const auto &[node, ty]: result.GetNodeTypes()) {
        if (ty.type == T_INT && node->Type() == SyntaxTreeType::Exp) {
            auto *e = static_cast<SyntaxTreeExp *>(const_cast<SyntaxTreeInterface *>(node));
            if (e->GetExpKind() == ExpKind::kPrefixExp) {
                found_field_access = true;
            }
        }
    }
    EXPECT_TRUE(found_field_access);
}

// ─────────────────────────────────────────────────────────────────────────
// 流敏感类型测试
// ─────────────────────────────────────────────────────────────────────────

// if-else 分支合并不同类型 → Meet 退化
TEST(pipeline, flow_sensitive_merge) {
    auto result = RunPipeline("./pipeline/test_flow_sensitive.lua");

    // 最终返回值的类型应为 T_FLOAT (meet of int and float)
    bool found_result = false;
    for (const auto &[node, ty]: result.GetNodeTypes()) {
        if (ty.type == T_FLOAT) {
            found_result = true;
            break;
        }
    }
    EXPECT_TRUE(found_result);
}

// ─────────────────────────────────────────────────────────────────────────
// 转译为 C 代码测试
// ─────────────────────────────────────────────────────────────────────────

// struct typedef 生成
TEST(pipeline, cgen_shape_struct) {
    auto result = RunPipeline("./pipeline/test_shape_literal.lua");

    const std::string code = result.GetRecordedCCode();

    // 应生成 LuaShapeN typedef
    EXPECT_NE(code.find("LuaShape"), std::string::npos) << "should generate LuaShape struct typedef\n" + code;

    // 应包含字段声明 (C 语法)
    EXPECT_NE(code.find("CVar b"), std::string::npos);
    EXPECT_NE(code.find("CVar c"), std::string::npos);
}

// ─────────────────────────────────────────────────────────────────────────
// 逃逸分析测试
// ─────────────────────────────────────────────────────────────────────────

// return 的变量标记为逃逸
TEST(pipeline, escape_return) {
    auto result = RunPipeline("./pipeline/test_escape.lua");

    // 逃逸分析结果中, 变量 'a' 应该标记为逃逸
    bool found_escape = false;
    for (const auto &[func, escapes]: result.GetEscapeVars()) {
        for (const auto &[var, is_escaped]: escapes) {
            if (var == "a" && is_escaped) {
                found_escape = true;
                break;
            }
        }
    }
    EXPECT_TRUE(found_escape) << "variable 'a' in test_escape should escape";
}
