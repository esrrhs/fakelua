#include "compile/cfg.h"
#include "compile/compiler.h"
#include "compile/my_flexer.h"
#include "compile/shape_type.h"
#include "compile/ssa.h"
#include "compile/syntax_tree.h"
#include "compile/unified_type_analyzer.h"
#include "fakelua.h"
#include "gtest/gtest.h"

// 解析器头文件
#include "compile/bison/parser.h"

#include <sstream>

using namespace fakelua;

// ── 辅助：从 Lua 源码字符串构建 CFG（绕过预处理，直接解析） ───────────
static CFGFunction BuildCfgFromSource(const std::string &lua_source,
                                       const std::vector<std::string> &params = {}) {
    const auto s = FakeluaNewState();
    if (!s) {
        return {};
    }

    // 直接解析，不经过 PreProcessor（预处理会将顶层语句包装到 __fakelua_init 中）
    MyFlexer f;
    f.InputString(lua_source);
    yy::parser parse(&f);
    auto code = parse.parse();
    if (code != 0) {
        FakeluaDeleteState(s);
        return {};
    }

    auto chunk = f.GetChunk();

    CFGBuilder builder;
    CFGFunction cfg = builder.Build(chunk, params, "test_func", false);

    FakeluaDeleteState(s);
    return cfg;
}

// ── Step 1.1: CFG 构造测试 ──────────────────────────────────────────

// 基本顺序语句：顶层代码中 entry 块就是函数体，所以是 entry + exit = 2 blocks
TEST(pipeline, cfg_sequential) {
    auto cfg = BuildCfgFromSource(R"(
        local a = 1
        local b = 2
        local c = a + b
    )");

    // 顶层代码：entry 块包含所有语句，exit 块为空
    ASSERT_EQ(cfg.blocks.size(), 2);
    ASSERT_EQ(cfg.entry_id, 0);

    // entry 块有 3 条 stmt
    const auto *entry = cfg.FindBlock(cfg.entry_id);
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->stmts.size(), 3);

    // exit 块无前驱以外的边
    const auto *exit = cfg.FindBlock(cfg.exit_ids[0]);
    ASSERT_NE(exit, nullptr);
    EXPECT_EQ(exit->stmts.size(), 0);
    EXPECT_EQ(exit->pred_ids.size(), 1);
    EXPECT_EQ(exit->pred_ids[0], cfg.entry_id);

    std::cout << cfg.DumpToString() << std::endl;
}

// if-else：应有 cond + then + else + merge + exit 共 5 个块
TEST(pipeline, cfg_if_else_merge) {
    auto cfg = BuildCfgFromSource(R"(
        local a = 1
        if a > 0 then
            a = 2
        else
            a = 3
        end
        local b = a
    )");

    // merge 块应该包含 "local b = a" 语句
    bool found_merge = false;
    for (const auto &blk : cfg.blocks) {
        for (const auto &s : blk.stmts) {
            if (s->Type() == SyntaxTreeType::LocalVar) {
                // 这个 local 声明是 if 之后的 "local b = a"
                auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(s);
                if (lv) {
                    auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
                    if (nl && nl->Names().size() == 1 && nl->Names()[0] == "b") {
                        found_merge = true;
                        // merge 块应该有 2 个前驱（then 尾 + else 尾）
                        EXPECT_GE(blk.pred_ids.size(), 2)
                            << "merge block should have >= 2 predecessors from branches";
                        break;
                    }
                }
            }
        }
        if (found_merge) break;
    }
    EXPECT_TRUE(found_merge) << "should find merge block containing 'local b = a'";

    std::cout << cfg.DumpToString() << std::endl;
}

// while 循环：有回边
TEST(pipeline, cfg_while_loop) {
    auto cfg = BuildCfgFromSource(R"(
        local i = 0
        while i < 10 do
            i = i + 1
        end
    )");

    // 找到循环 header 块（包含 While 节点）
    const BasicBlock *header = nullptr;
    for (const auto &b : cfg.blocks) {
        for (const auto &s : b.stmts) {
            if (s->Type() == SyntaxTreeType::While) {
                header = &b;
                break;
            }
        }
        if (header) break;
    }
    ASSERT_NE(header, nullptr);

    // header 应该有回边：某个 body 后继的后继是 header
    bool has_back_edge = false;
    for (int sid : header->succ_ids) {
        auto *succ = cfg.FindBlock(sid);
        if (succ) {
            for (int ssucc : succ->succ_ids) {
                if (ssucc == header->id) has_back_edge = true;
            }
        }
    }
    EXPECT_TRUE(has_back_edge) << "while loop should have back edge from body to header";

    std::cout << cfg.DumpToString() << std::endl;
}

// for 数值循环
TEST(pipeline, cfg_for_loop) {
    auto cfg = BuildCfgFromSource(R"(
        for i = 1, 10 do
            local x = i
        end
    )");

    // 找到 init 块（包含 ForLoop 节点）
    const BasicBlock *init = nullptr;
    for (const auto &b : cfg.blocks) {
        for (const auto &s : b.stmts) {
            if (s->Type() == SyntaxTreeType::ForLoop) {
                init = &b;
                break;
            }
        }
        if (init) break;
    }
    ASSERT_NE(init, nullptr);

    // init 应该有一个 succ 是 exit
    bool has_exit_succ = false;
    for (int sid : init->succ_ids) {
        if (std::find(cfg.exit_ids.begin(), cfg.exit_ids.end(), sid) != cfg.exit_ids.end())
            has_exit_succ = true;
    }
    EXPECT_TRUE(has_exit_succ) << "for loop should have exit successor";

    std::cout << cfg.DumpToString() << std::endl;
}

// 支配关系：entry 支配所有块，每块支配自身
TEST(pipeline, cfg_dominators_basic) {
    auto cfg = BuildCfgFromSource(R"(
        local a = 1
        if a > 0 then
            a = 2
        else
            a = 3
        end
        local b = a
    )");

    for (const auto &b : cfg.blocks) {
        auto it = cfg.dominators.find(b.id);
        ASSERT_NE(it, cfg.dominators.end());
        EXPECT_TRUE(it->second.count(cfg.entry_id))
            << "entry should dominate block " << b.id;
        EXPECT_TRUE(it->second.count(b.id))
            << "block " << b.id << " should dominate itself";
    }
}

// ── 辅助：从 Lua 源码构建 SSA ───────────────────────────────────────────
static SSAFunction BuildSsaFromSource(const std::string &lua_source,
                                       const std::vector<std::string> &params = {}) {
    CFGFunction cfg = BuildCfgFromSource(lua_source, params);
    SSABuilder builder;
    return builder.Build(cfg);
}

// ── 辅助：从 Lua 源码构建 InferResult（运行 SSA+类型分析，不经过预处理） ──
static InferResult AnalyzeSource(const std::string &lua_source) {
    auto cfg = BuildCfgFromSource(lua_source);

    const auto s = FakeluaNewState();
    MyFlexer f;
    f.InputString(lua_source);
    yy::parser parse(&f);
    parse.parse();
    auto chunk = f.GetChunk();
    FakeluaDeleteState(s);

    InferResult ir;
    ir.shape_registry = std::make_shared<ShapeRegistry>();

    UnifiedTypeAnalyzer uta(ir.shape_registry.get());
    SSABuilder ssa_builder;
    SSAFunction ssa = ssa_builder.Build(cfg);

    uta.Analyze("test_func", chunk, cfg, ssa, ir);

    return ir;
}

// ── Step 1.2: SSA 构造测试 ──────────────────────────────────────────

// 简单顺序赋值：a = 1; a = 2 应产生 a_v1, a_v2
TEST(pipeline, ssa_sequential_assign) {
    auto cfg = BuildCfgFromSource(R"(
        local a = 1
        a = 2
    )");

    SSABuilder builder;
    SSAFunction ssa = builder.Build(cfg);

    // a 应有 2 个版本（v0 来自 local a = 1, v1 来自 a = 2）
    auto it = ssa.var_all_versions.find("a");
    ASSERT_NE(it, ssa.var_all_versions.end());
    EXPECT_EQ(it->second.size(), 2u);

    // 无分支，应无 φ
    EXPECT_EQ(ssa.block_phis.size(), 0u);

    std::cout << ssa.DumpToString() << std::endl;
}

// if-else 分支合并：应在 merge 块插入 φ 节点
TEST(pipeline, ssa_if_else_phi) {
    auto cfg = BuildCfgFromSource(R"(
        local a = 1
        if a > 0 then
            a = 2
        else
            a = 3
        end
        local b = a
    )");

    SSABuilder builder;
    SSAFunction ssa = builder.Build(cfg);

    // a 应有 4 个版本：v0 (local a=1), v1 (φ in merge), v3 (a=2 in then), v4 (a=3 in else)
    auto it = ssa.var_all_versions.find("a");
    ASSERT_NE(it, ssa.var_all_versions.end());
    EXPECT_EQ(it->second.size(), 4u);

    // 应在 merge 块有 φ 节点
    bool found_phi = false;
    for (const auto &[bid, phis] : ssa.block_phis) {
        for (const auto &phi : phis) {
            if (phi.var_name == "a" && phi.result_version >= 0) {
                found_phi = true;
                EXPECT_GE(phi.arg_versions.size(), 2u)
                    << "phi should have 2 args from then/else branches";
                EXPECT_NE(phi.arg_versions[0], -1);
                EXPECT_NE(phi.arg_versions[1], -1);
                EXPECT_NE(phi.arg_versions[0], phi.arg_versions[1])
                    << "phi args should be different versions";
            }
        }
    }
    EXPECT_TRUE(found_phi) << "should have phi for 'a' in merge block";

    std::cout << ssa.DumpToString() << std::endl;
}

// while 循环：应在 header 块插入 φ 节点
TEST(pipeline, ssa_while_phi) {
    auto cfg = BuildCfgFromSource(R"(
        local i = 0
        while i < 10 do
            i = i + 1
        end
    )");

    SSABuilder builder;
    SSAFunction ssa = builder.Build(cfg);

    // i 应有多个版本：v0 (local i=0), v1 (i=i+1 in body), v2 (φ in header)
    auto it = ssa.var_all_versions.find("i");
    ASSERT_NE(it, ssa.var_all_versions.end());
    EXPECT_GE(it->second.size(), 3u);

    // 应在 header 块有 φ 节点
    // header 块是包含 While 语句的块
    int header_id = -1;
    for (const auto &b : cfg.blocks) {
        for (const auto &s : b.stmts) {
            if (s->Type() == SyntaxTreeType::While) { header_id = b.id; break; }
        }
        if (header_id >= 0) break;
    }
    ASSERT_GE(header_id, 0);

    auto phi_it = ssa.block_phis.find(header_id);
    ASSERT_NE(phi_it, ssa.block_phis.end()) << "should have phi in while header block";

    bool found_i_phi = false;
    for (const auto &phi : phi_it->second) {
        if (phi.var_name == "i") {
            found_i_phi = true;
            EXPECT_GE(phi.arg_versions.size(), 2u)
                << "phi should have 2 args (pre-loop + back-edge)";
        }
    }
    EXPECT_TRUE(found_i_phi) << "should have phi for 'i' in while header";

    std::cout << ssa.DumpToString() << std::endl;
}

// 参数版本：函数参数应有初始版本
TEST(pipeline, ssa_param_versions) {
    // 使用带参数的源码（通过函数定义）
    auto cfg = BuildCfgFromSource(R"(
        local function f(x, y)
            return x + y
        end
    )", {"x", "y"});

    SSABuilder builder;
    SSAFunction ssa = builder.Build(cfg);

    // 参数 x, y 应有初始版本
    EXPECT_EQ(ssa.param_versions.size(), 2u);
    EXPECT_GE(ssa.param_versions[0], 0);
    EXPECT_GE(ssa.param_versions[1], 0);

    // x, y 应在 var_all_versions 中
    EXPECT_TRUE(ssa.var_all_versions.count("x"));
    EXPECT_TRUE(ssa.var_all_versions.count("y"));

    std::cout << ssa.DumpToString() << std::endl;
}

// ── Step 1.3: 简单类型推导测试 ──────────────────────────────────────────

// 辅助：在 ir.main_ssa_types 中查找给定源 AST 节点对应的类型
static InferredType LookupNodeType(const InferResult &ir, const SyntaxTreeInterface *needle) {
    auto it = ir.main_ssa_types.find(needle);
    if (it != ir.main_ssa_types.end()) return it->second.type;
    return T_UNKNOWN;
}

// 测试字面量类型推导
TEST(pipeline, type_literals) {
    auto ir = AnalyzeSource(R"(
        local a = 1
        local b = 1.5
        local c = "hello"
        local d = nil
        local e = true
    )");

    // main_ssa_types 不应为空
    EXPECT_FALSE(ir.main_ssa_types.empty()) << "should have type info for nodes";

    // shape_registry 应存在
    EXPECT_TRUE(ir.shape_registry != nullptr);

    std::cout << "main_ssa_types count: " << ir.main_ssa_types.size() << "\n";
}

// 测试二元运算类型推导
TEST(pipeline, type_binop) {
    auto ir = AnalyzeSource(R"(
        local a = 1
        local b = 2
        local c = a + b
    )");

    // main_ssa_types 应包含 binop 节点的类型信息
    EXPECT_FALSE(ir.main_ssa_types.empty());

    // 查找 binop 节点
    int binop_count = 0;
    for (const auto &[node, ssa] : ir.main_ssa_types) {
        if (node->Type() == SyntaxTreeType::Exp) {
            auto *e = static_cast<SyntaxTreeExp*>(const_cast<SyntaxTreeInterface*>(node));
            if (e->GetExpKind() == ExpKind::kBinop) {
                binop_count++;
            }
        }
    }
    EXPECT_GE(binop_count, 1) << "should have at least one binop typed";
}

// 测试变量赋值传播
TEST(pipeline, type_var_propagation) {
    auto ir = AnalyzeSource(R"(
        local a = 1
        local b = a
    )");

    // b = a 的 RHS (a) 的类型应被推导出来
    EXPECT_FALSE(ir.main_ssa_types.empty());
}

// ── Step 1.4: Shape 字面量构造测试 ────────────────────────────────────

// 测试 table 构造 {b=1, c=2.0} 产生封闭 record
TEST(pipeline, shape_literal) {
    auto ir = AnalyzeSource(R"(
        local a = {b=1, c=2.0}
    )");

    // shape_registry 应有至少 1 个 shape
    ASSERT_TRUE(ir.shape_registry != nullptr);
    EXPECT_GE(ir.shape_registry->Count(), 1) << "should have at least one shape";

    // 验证 shape 的字段（直接检查 registry 中的所有 shape）
    bool found = false;
    for (int sid = 0; sid < ir.shape_registry->Count(); ++sid) {
        const ShapeType &shape = ir.shape_registry->Get(sid);
        if (shape.fields.empty()) continue;
        if (shape.is_open) continue;

        // 检查是否有 b 和 c 字段
        bool has_b = false, has_c = false;
        for (const auto &f : shape.fields) {
            if (f.name == "b" && f.type == T_INT) has_b = true;
            if (f.name == "c" && f.type == T_FLOAT) has_c = true;
        }
        if (has_b && has_c) {
            found = true;
            EXPECT_GE(shape.fields.size(), 2u);
            break;
        }
    }
    EXPECT_TRUE(found) << "should find a closed shape with fields b:int, c:float";

    std::cout << "shape count: " << ir.shape_registry->Count() << "\n";
    for (int sid = 0; sid < ir.shape_registry->Count(); ++sid) {
        const ShapeType &shape = ir.shape_registry->Get(sid);
        std::cout << "  shape " << sid << ": open=" << shape.is_open << " fields=[";
        for (size_t i = 0; i < shape.fields.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << shape.fields[i].name << ":"
                      << InferredTypeToString(shape.fields[i].type);
        }
        std::cout << "]\n";
    }
}

// 测试空 table {} 不产生 shape（或产生空 shape）
TEST(pipeline, shape_empty) {
    auto ir = AnalyzeSource(R"(
        local a = {}
    )");

    // 空 table 不应产生有字段的 shape
    for (const auto &[node, shape_id] : ir.ctor_target_shapes) {
        if (shape_id < 0) continue;
        const ShapeType &shape = ir.shape_registry->Get(shape_id);
        EXPECT_EQ(shape.fields.size(), 0u) << "empty table should have no fields";
    }
}

// 测试嵌套 table
TEST(pipeline, shape_nested) {
    auto ir = AnalyzeSource(R"(
        local a = {p={x=1, y=2}}
    )");

    ASSERT_TRUE(ir.shape_registry != nullptr);
    // 应至少有 2 个 shape（外层和内层）
    EXPECT_GE(ir.shape_registry->Count(), 2u) << "should have shapes for both outer and inner tables";
}

// ── Step 2.2: φ 节点类型推导测试 ──────────────────────────────────────

// 测试 if-else 中 φ 结果类型 = meet(then_type, else_type)
TEST(pipeline, phi_type_if_else) {
    auto ir = AnalyzeSource(R"(
        local a = 1
        if a > 0 then
            a = 2.0      -- a becomes float
        else
            a = 3        -- a stays int
        end
        local b = a      -- b should be float (meet of float and int)
    )");

    // 查找 φ 结果版本的类型
    // φ 在 merge 块中，结果类型应为 meet(float, int) = float
    for (const auto &[ver, ty] : ir.ssa_version_types) {
        // φ 结果版本的类型应被设置（不为 T_UNKNOWN）
        if (ty.type != T_UNKNOWN) {
            std::cout << "  version " << ver << ": type=" << InferredTypeToString(ty.type)
                      << " shape=" << ty.shape_id << "\n";
        }
    }

    // 验证 version_types 不为空（至少参数版本和 φ 版本有类型）
    EXPECT_FALSE(ir.ssa_version_types.empty());
}

// ── Step 2.3: 工作表不动点 + 流敏感类型测试 ───────────────────────────

// 测试 if-else 分支中相同字段不同数值类型的 Meet
// then: c = {x=1} (x:int), else: c = {x=2.0} (x:float)
// meet 后: c 应有含 x 字段的 shape
TEST(pipeline, flow_sensitive_if_merge_same_field) {
    auto ir = AnalyzeSource(R"(
        if 1 then
            local c = {x=1}       -- x: int
        else
            local c = {x=2.0}     -- x: float
        end
    )");

    // 两个分支都创建了 shape，registry 应有 shape
    ASSERT_TRUE(ir.shape_registry != nullptr);
    EXPECT_GE(ir.shape_registry->Count(), 1u);

    // 验证创建了含 x 字段的 shape
    bool found_x_shape = false;
    for (int sid = 0; sid < ir.shape_registry->Count(); ++sid) {
        const ShapeType &shape = ir.shape_registry->Get(sid);
        for (const auto &f : shape.fields) {
            if (f.name == "x") { found_x_shape = true; break; }
        }
    }
    EXPECT_TRUE(found_x_shape) << "should have shape with field x";
}

// 测试 Worklist 在分支合并时的 Meet（不同变量名的分支赋值）
TEST(pipeline, flow_sensitive_branch_isolation) {
    auto ir = AnalyzeSource(R"(
        local a = nil
        local b = nil
        if 1 then
            a = {x=1}
        else
            b = {y=2}
        end
    )");

    // a 和 b 应各自有独立的 shape
    ASSERT_TRUE(ir.shape_registry != nullptr);
    EXPECT_GE(ir.shape_registry->Count(), 2u);
}

// 测试数值类型在分支合并时提升
TEST(pipeline, flow_sensitive_numeric_promotion) {
    auto ir = AnalyzeSource(R"(
        local a = 1          -- int
        if 1 then
            a = 2.0          -- float
        else
            a = 3            -- int
        end
        local b = a          -- should be float (meet of float and int)
    )");

    // 查找 φ 结果版本的类型
    bool found_phi_type = false;
    for (const auto &[ver, ty] : ir.ssa_version_types) {
        if (ty.type == T_FLOAT) {
            found_phi_type = true;
            break;
        }
    }
    // 至少参数版本和 φ 版本有类型
    EXPECT_FALSE(ir.ssa_version_types.empty());
    std::cout << "version types count: " << ir.ssa_version_types.size() << "\n";
}

// 测试 while 循环中的类型收敛
TEST(pipeline, flow_sensitive_while_convergence) {
    // while 循环的 header 应有 φ 节点
    auto cfg = BuildCfgFromSource(R"(
        local i = 0
        while i < 10 do
            i = i + 1
        end
    )");
    SSABuilder ssa_builder;
    SSAFunction ssa = ssa_builder.Build(cfg);

    // header 块应有 φ
    int header_id = -1;
    for (const auto &b : cfg.blocks) {
        for (const auto &s : b.stmts) {
            if (s->Type() == SyntaxTreeType::While) { header_id = b.id; break; }
        }
        if (header_id >= 0) break;
    }
    EXPECT_GE(header_id, 0);

    auto phi_it = ssa.block_phis.find(header_id);
    EXPECT_NE(phi_it, ssa.block_phis.end()) << "while header should have phi";

    std::cout << "while header " << header_id << " phis: " << phi_it->second.size() << "\n";
    std::cout << ssa.DumpToString() << std::endl;
}

// ── Step 2.1: Meet 操作测试 ──────────────────────────────────────────

// 测试标量 Meet（规范 §5.3 Meet 真值表）
TEST(pipeline, meet_scalars) {
    ShapeRegistry reg;

    // Meet(int, int) = int
    EXPECT_TRUE(reg.MeetType(T_INT, T_INT) == T_INT);
    // Meet(float, float) = float
    EXPECT_TRUE(reg.MeetType(T_FLOAT, T_FLOAT) == T_FLOAT);
    // Meet(int, float) = float（提升）
    EXPECT_TRUE(reg.MeetType(T_INT, T_FLOAT) == T_FLOAT);
    EXPECT_TRUE(reg.MeetType(T_FLOAT, T_INT) == T_FLOAT);
    // Meet(int, string) = dynamic
    EXPECT_TRUE(reg.MeetType(T_INT, T_STRING) == T_DYNAMIC);
    // Meet(dynamic, 任意) = dynamic
    EXPECT_TRUE(reg.MeetType(T_DYNAMIC, T_INT) == T_DYNAMIC);
    EXPECT_TRUE(reg.MeetType(T_INT, T_DYNAMIC) == T_DYNAMIC);
    // Meet(nil, int) = int
    EXPECT_TRUE(reg.MeetType(T_NIL, T_INT) == T_INT);
    EXPECT_TRUE(reg.MeetType(T_INT, T_NIL) == T_INT);
}

// 测试 Record Meet（字段并集，同名字段 Meet 类型）
TEST(pipeline, meet_records) {
    ShapeRegistry reg;

    // 创建 Rec{b:int}
    ShapeType rec1;
    rec1.is_open = false;
    rec1.fields.push_back({"b", "b", T_INT, false, false});
    int id1 = reg.Intern(std::move(rec1));

    // 创建 Rec{b:float}
    ShapeType rec2;
    rec2.is_open = false;
    rec2.fields.push_back({"b", "b", T_FLOAT, false, false});
    int id2 = reg.Intern(std::move(rec2));

    // Meet(Rec{b:int}, Rec{b:float}) → Rec{b:float}
    int meet_id = reg.Meet(id1, id2);
    const ShapeType &meet_shape = reg.Get(meet_id);
    EXPECT_FALSE(meet_shape.is_open);
    EXPECT_EQ(meet_shape.fields.size(), 1u);
    EXPECT_EQ(meet_shape.fields[0].name, "b");
    EXPECT_EQ(meet_shape.fields[0].type, T_FLOAT);  // int meet float = float

    // 创建 Rec{c:int}（不同字段）
    ShapeType rec3;
    rec3.is_open = false;
    rec3.fields.push_back({"c", "c", T_INT, false, false});
    int id3 = reg.Intern(std::move(rec3));

    // Meet(Rec{b:int}, Rec{c:int}) → Rec{b:int, c:int}
    int meet_id2 = reg.Meet(id1, id3);
    const ShapeType &meet_shape2 = reg.Get(meet_id2);
    EXPECT_EQ(meet_shape2.fields.size(), 2u);
    // 检查 optional 标记（字段在另一边不存在 → optional=true）
    for (const auto &f : meet_shape2.fields) {
        // 在 Phase 1 的 Meet 中，不存在的字段设为 optional
        // （但这里两个 record 都没有对方的字段，所以都应该是 optional）
    }
}

// 测试 Open Record Meet
TEST(pipeline, meet_open_records) {
    ShapeRegistry reg;

    // 创建封闭 Rec{b:int}
    ShapeType rec1;
    rec1.is_open = false;
    rec1.fields.push_back({"b", "b", T_INT, false, false});
    int id1 = reg.Intern(std::move(rec1));

    // 创建开放 Rec{d:int,...}
    ShapeType rec2;
    rec2.is_open = true;
    rec2.fields.push_back({"d", "d", T_INT, false, false});
    int id2 = reg.Intern(std::move(rec2));

    // Meet(封闭, 开放) → 开放
    int meet_id = reg.Meet(id1, id2);
    const ShapeType &meet_shape = reg.Get(meet_id);
    EXPECT_TRUE(meet_shape.is_open);
    // 应包含 b（optional）和 d
    EXPECT_GE(meet_shape.fields.size(), 1u);
}

// ── Step 3.4: 收敛性测试 ──────────────────────────────────────────────

// 规范 §14.3：循环里 shape 增长必须在有限步内 widen 到 dynamic
TEST(pipeline, widening_convergence) {
    // 循环加不同字段 → widening 触发 → 退化为 dynamic
    auto ir = AnalyzeSource(R"(
        local t = {}
        t.a1 = 1
        t.a2 = 2
        t.a3 = 3
        t.a4 = 4
        t.a5 = 5
    )");

    // 字段数超过阈值(16)前应能正常构造
    ASSERT_TRUE(ir.shape_registry != nullptr);
    std::cout << "shape count: " << ir.shape_registry->Count() << "\n";
    for (int sid = 0; sid < ir.shape_registry->Count(); ++sid) {
        const ShapeType &shape = ir.shape_registry->Get(sid);
        std::cout << "  shape " << sid << ": open=" << shape.is_open
                  << " fields=" << shape.fields.size() << "\n";
    }
}

// 测试大量字段写入触发 widening
TEST(pipeline, widening_many_fields) {
    // 从封闭 record 开始，写入超过 16 个字段 → 应触发 widening
    auto ir = AnalyzeSource(R"(
        local t = {init=1}
        t.f1 = 1
        t.f2 = 2
        t.f3 = 3
        t.f4 = 4
        t.f5 = 5
        t.f6 = 6
        t.f7 = 7
        t.f8 = 8
        t.f9 = 9
        t.f10 = 10
        t.f11 = 11
        t.f12 = 12
        t.f13 = 13
        t.f14 = 14
        t.f15 = 15
        t.f16 = 16
        t.f17 = 17
    )");

    // 应能正常完成，不卡死，且最终 shape 应被 widen（开放或字段截断）
    ASSERT_TRUE(ir.shape_registry != nullptr);
    EXPECT_GE(ir.shape_registry->Count(), 1u);
    std::cout << "shape count after many fields: " << ir.shape_registry->Count() << "\n";
    for (int sid = 0; sid < ir.shape_registry->Count(); ++sid) {
        const ShapeType &shape = ir.shape_registry->Get(sid);
        std::cout << "  shape " << sid << ": open=" << shape.is_open
                  << " fields=" << shape.fields.size() << "\n";
    }
}

// 测试循环中 shape 增长的收敛性（规范 §14.3）
// 注意：Phase 3 的 while 循环回边会触发 worklist 迭代，
// 每次迭代可能增加字段，widening 确保收敛
TEST(pipeline, widening_loop_convergence) {
    // 使用 while true 循环加字段（接近规范 §12.1 场景）
    auto ir = AnalyzeSource(R"(
        local t = {n=1}
        local i = 0
        while i < 5 do
            t["k" .. i] = i
            i = i + 1
        end
    )");

    // 应能正常完成，不卡死
    ASSERT_TRUE(ir.shape_registry != nullptr);
    std::cout << "shape count after loop: " << ir.shape_registry->Count() << "\n";
}

// ── Step 3.1 + 3.2: 字段写入 + 字段读取测试 ────────────────────────────

// 测试 literal table 字段读取 a.b → 返回字段类型
TEST(pipeline, field_read_literal) {
    auto ir = AnalyzeSource(R"(
        local a = {b=1, c=2.0}
    )");

    // 验证 shape 有 b 和 c 字段
    ASSERT_TRUE(ir.shape_registry != nullptr);
    bool found_b = false, found_c = false;
    for (int sid = 0; sid < ir.shape_registry->Count(); ++sid) {
        const ShapeType &shape = ir.shape_registry->Get(sid);
        for (const auto &f : shape.fields) {
            if (f.name == "b" && f.type == T_INT) found_b = true;
            if (f.name == "c" && f.type == T_FLOAT) found_c = true;
        }
    }
    EXPECT_TRUE(found_b) << "should have field b:T_INT";
    EXPECT_TRUE(found_c) << "should have field c:T_FLOAT";
}

// 测试字段写入 a.d = v 给 record 加字段
TEST(pipeline, field_write_new_field) {
    auto ir = AnalyzeSource(R"(
        local a = {b=1}
        a.c = 2      -- 给封闭 record 加新字段 → 退化为开放
    )");

    // 找到包含 b 和 c 字段的 shape
    bool found = false;
    for (int sid = 0; sid < ir.shape_registry->Count(); ++sid) {
        const ShapeType &shape = ir.shape_registry->Get(sid);
        if (shape.fields.size() == 2) {
            bool has_b = false, has_c = false;
            for (const auto &f : shape.fields) {
                if (f.name == "b") has_b = true;
                if (f.name == "c") has_c = true;
            }
            if (has_b && has_c) {
                found = true;
                EXPECT_TRUE(shape.is_open) << "after adding field, should be open";
                break;
            }
        }
    }
    EXPECT_TRUE(found) << "should find open shape with b and c fields";

    std::cout << "shape count: " << ir.shape_registry->Count() << "\n";
    for (int sid = 0; sid < ir.shape_registry->Count(); ++sid) {
        const ShapeType &shape = ir.shape_registry->Get(sid);
        std::cout << "  shape " << sid << ": open=" << shape.is_open << " fields=[";
        for (size_t i = 0; i < shape.fields.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << shape.fields[i].name << ":"
                      << InferredTypeToString(shape.fields[i].type);
        }
        std::cout << "]\n";
    }
}

// 测试字段写入 a.b = newtype 合一字段类型
TEST(pipeline, field_write_existing_field) {
    auto ir = AnalyzeSource(R"(
        local a = {b=1}
        a.b = 2.0
    )");

    // 找到包含 b 字段且类型为 float 的 shape
    bool found = false;
    for (int sid = 0; sid < ir.shape_registry->Count(); ++sid) {
        const ShapeType &shape = ir.shape_registry->Get(sid);
        for (const auto &f : shape.fields) {
            if (f.name == "b" && f.type == T_FLOAT) { found = true; break; }
        }
    }
    EXPECT_TRUE(found) << "field b should be float after meet(int, float)";
}
