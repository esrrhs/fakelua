#include "fakelua.h"
#include "gtest/gtest.h"
#include <unordered_map>

using namespace fakelua;

TEST(test_native, test_basic_kv) {
    auto* obj = NativeObject::Create("player");
    obj->SetInt("hp", 100);
    obj->SetFloat("speed", 5.5);
    obj->SetBool("alive", true);
    obj->SetString("name", "hero");

    EXPECT_EQ(obj->GetInt("hp"), 100);
    EXPECT_DOUBLE_EQ(obj->GetFloat("speed"), 5.5);
    EXPECT_TRUE(obj->GetBool("alive"));
    EXPECT_EQ(obj->GetString("name"), "hero");
    EXPECT_EQ(obj->GetTypeName(), "player");
    EXPECT_EQ(obj->Size(), 4);

    NativeObject::Destroy(obj);
}

TEST(test_native, test_nested_object) {
    auto* player = NativeObject::Create("player");
    auto* bag = NativeObject::Create("bag");

    bag->SetInt("gold", 999);
    player->SetObject("bag", bag);

    EXPECT_EQ(player->GetObject("bag"), bag);
    EXPECT_EQ(player->GetObject("bag")->GetInt("gold"), 999);

    NativeObject::Destroy(bag);
    NativeObject::Destroy(player);
}

TEST(test_native, test_lua_integration_get_set) {
    auto* s = FakeluaNewState();

    static NativeObject* global_player = nullptr;
    global_player = NativeObject::Create("player");
    global_player->SetInt("hp", 100);

    RegisterNativeFunction(s, "get_player", 0, false,
        [](State* state, CVar* args, int n) -> CVar {
            return global_player->Wrap(state);
        });

    CompileConfig config;
    CompileFile(s, "./native/test_native_get_set.lua", config);

    CVar ret;
    Call(s, JIT_TCC, "test_get_set", ret);

    EXPECT_EQ(global_player->GetInt("hp"), 150);

    NativeObject::Destroy(global_player);
    global_player = nullptr;
    FakeluaDeleteState(s);
}

TEST(test_native, test_fully_dynamic_property_and_builtin_api) {
    auto* s = FakeluaNewState();

    CompileConfig config;
    CompileFile(s, "./native/test_native_on_msg.lua", config);

    // ── 1. 模拟登录：Call("on_msg", "on_login", 1001, "Alice") ────────────────
    CVar login_ret;
    Call(s, JIT_TCC, "on_msg", login_ret, "on_login", 1001, "Alice");
    int64_t initial_hp = inter::FakeluaToNative<int64_t>(s, login_ret);
    EXPECT_EQ(initial_hp, 100);

    // 校验 C++ 全局管理器中的 NativeObject 实例属性
    NativeObject* alice = NativeObjectManager::Instance().Get("player", 1001);
    ASSERT_NE(alice, nullptr);
    EXPECT_EQ(alice->GetInt("hp"), 100);
    EXPECT_EQ(alice->GetInt("mp"), 200);
    EXPECT_EQ(alice->GetString("name"), "Alice");

    // ── 2. 模拟跨帧 reset（Arena 内存重置）─────────────────────────────────
    inter::Reset(s);

    // ── 3. 模拟对话：Call("on_msg", "on_talk", 1001, "Hello fakelua!") ─────────
    CVar talk_ret;
    Call(s, JIT_TCC, "on_msg", talk_ret, "on_talk", 1001, "Hello fakelua!");
    std::string talk_res = inter::FakeluaToNative<std::string>(s, talk_ret);
    EXPECT_EQ(talk_res, "Alice (80hp): Hello fakelua!");

    // 校验 C++ 侧持久数据：hp 扣减为 80，动态字段 last_talk 成功设置
    EXPECT_EQ(alice->GetInt("hp"), 80);
    EXPECT_EQ(alice->GetString("last_talk"), "Hello fakelua!");

    // 清理全局 NativeObjectManager
    NativeObjectManager::Instance().Clear();
    FakeluaDeleteState(s);
}

TEST(test_native, test_lua_nested_object) {
    auto* s = FakeluaNewState();

    CompileConfig config;
    CompileFile(s, "./native/test_native_nested.lua", config);

    // ── 1. 执行 test_nested()，在 Lua 中创建 player 与 bag 并绑定嵌套 ────────
    CVar ret1;
    Call(s, JIT_TCC, "test_nested", ret1);
    int64_t sum = inter::FakeluaToNative<int64_t>(s, ret1);
    EXPECT_EQ(sum, 1049); // 999 + 50 = 1049

    // 校验 C++ 全局管理器与嵌套对象属性
    NativeObject* player = NativeObjectManager::Instance().Get("player", 2001);
    ASSERT_NE(player, nullptr);
    NativeObject* bag = player->GetObject("bag");
    ASSERT_NE(bag, nullptr);
    EXPECT_EQ(bag->GetInt("gold"), 999);
    EXPECT_EQ(bag->GetInt("capacity"), 50);

    // ── 2. 跨帧 Reset 内存 ─────────────────────────────────────────────────
    inter::Reset(s);

    // ── 3. 再次在 Lua 中通过 get_native_obj 获取 player 并读取 player.bag.gold ─
    CVar ret2;
    Call(s, JIT_TCC, "test_nested_fetch", ret2);
    int64_t remaining_gold = inter::FakeluaToNative<int64_t>(s, ret2);
    EXPECT_EQ(remaining_gold, 899); // 999 - 100 = 899

    // 校验 C++ 侧底层嵌套数据改变
    EXPECT_EQ(bag->GetInt("gold"), 899);

    NativeObjectManager::Instance().Clear();
    FakeluaDeleteState(s);
}

TEST(test_native, test_group_arena_batch_destroy) {
    auto* s = FakeluaNewState();

    CompileConfig config;
    CompileFile(s, "./native/test_native_group.lua", config);

    // ── 1. 在 Lua 中创建玩家 1001 以及归属于 1001 的 bag 和 item 对象 ─────────
    CVar ret1;
    Call(s, JIT_TCC, "test_group_create", ret1, 1001);
    std::string weapon_name = inter::FakeluaToNative<std::string>(s, ret1);
    EXPECT_EQ(weapon_name, "Excalibur");

    // 验证对象存在于管理器中
    EXPECT_NE(NativeObjectManager::Instance().Get("player", 1001), nullptr);
    EXPECT_NE(NativeObjectManager::Instance().Get("bag", 10010), nullptr);
    EXPECT_NE(NativeObjectManager::Instance().Get("item", 100100), nullptr);

    // ── 2. 一口气批处理销毁 1001 组下的所有 NativeObject ────────────────────
    CVar ret2;
    Call(s, JIT_TCC, "test_group_destroy", ret2, 1001);
    int64_t destroyed_count = inter::FakeluaToNative<int64_t>(s, ret2);
    EXPECT_EQ(destroyed_count, 3); // 3 个对象全部一口气清理

    // 验证销毁后这 3 个对象全部被移除
    EXPECT_EQ(NativeObjectManager::Instance().Get("player", 1001), nullptr);
    EXPECT_EQ(NativeObjectManager::Instance().Get("bag", 10010), nullptr);
    EXPECT_EQ(NativeObjectManager::Instance().Get("item", 100100), nullptr);

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_var_interface_callback) {
    auto* s = FakeluaNewState();

    // 注册以 VarInterface* 向量为入参的 C++ 回调，0 感知 CVar
    RegisterNativeVarFunction(s, "cpp_process_user", 1, false,
        NativeVarFuncCallback([](State* state, const std::vector<VarInterface*>& args) -> VarInterface* {
            EXPECT_GE(args.size(), 1);
            VarInterface* user_info = args[0];
            EXPECT_EQ(user_info->ViGetType(), VarInterface::Type::TABLE);

            int64_t score = 0;
            std::string name;
            for (size_t i = 0; i < user_info->ViGetTableSize(); ++i) {
                auto kv = user_info->ViGetTableKv(static_cast<int>(i));
                if (kv.first->ViGetString() == "name") {
                    name = kv.second->ViGetString();
                } else if (kv.first->ViGetString() == "score") {
                    score = kv.second->ViGetInt();
                }
            }

            auto* res = new SimpleVarImpl();
            res->ViSetString(name + " has score " + std::to_string(score + 100));
            return res;
        }));

    CompileConfig config;
    CompileString(s, R"(
        function run_test()
            local user = { name = "Bob", score = 50 }
            return cpp_process_user(user)
        end
    )", config);

    std::string result;
    Call(s, JIT_TCC, "run_test", result);
    EXPECT_EQ(result, "Bob has score 150");

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_typed_template_callback) {
    auto* s = FakeluaNewState();

    // 使用强类型 C++ 模板注册 lambda，完全零感知 CVar
    RegisterNativeFunction(s, "cpp_add_hp", false, std::function<int64_t(State*, int64_t, int64_t)>(
        [](State* state, int64_t hp, int64_t add) -> int64_t {
            return hp + add;
        }));

    CompileConfig config;
    CompileString(s, R"(
        function calc()
            return cpp_add_hp(120, 30)
        end
    )", config);

    int64_t res = 0;
    Call(s, JIT_TCC, "calc", res);
    EXPECT_EQ(res, 150);

    FakeluaDeleteState(s);
}
