#include "fakelua.h"
#include "gtest/gtest.h"
#include "var/var_type.h"
#include <unordered_map>

using namespace fakelua;

TEST(test_native, test_basic_kv) {
    int64_t gid = NativeObjectManager::Instance().CreateGroup(1);
    auto *obj = NativeObjectManager::Instance().Create(gid, "player", 1);
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

    NativeObjectManager::Instance().DestroyGroup(gid);
}

TEST(test_native, test_nested_object) {
    int64_t gid = NativeObjectManager::Instance().CreateGroup(2);
    auto *player = NativeObjectManager::Instance().Create(gid, "player", 1);
    auto *bag = NativeObjectManager::Instance().Create(gid, "bag", 2);

    bag->SetInt("gold", 999);
    player->SetObject("bag", bag);

    EXPECT_EQ(player->GetObject("bag"), bag);
    EXPECT_EQ(player->GetObject("bag")->GetInt("gold"), 999);

    NativeObjectManager::Instance().DestroyGroup(gid);
}

TEST(test_native, test_lua_integration_get_set) {
    auto *s = FakeluaNewState();

    int64_t gid = NativeObjectManager::Instance().CreateGroup(100);
    static NativeObject *global_player = nullptr;
    global_player = NativeObjectManager::Instance().Create(gid, "player", 100);
    global_player->SetInt("hp", 100);

    RegisterNativeFunction(s, "get_player", 0, false, [](State *state, CVar *args, int n) -> CVar { return inter::NativeToFakelua(state, global_player); });

    CompileConfig config;
    CompileFile(s, "./native/test_native_get_set.lua", config);

    CVar ret;
    Call(s, JIT_TCC, "test_get_set", ret);

    EXPECT_EQ(global_player->GetInt("hp"), 150);

    NativeObjectManager::Instance().DestroyGroup(gid);
    global_player = nullptr;
    FakeluaDeleteState(s);
}

TEST(test_native, test_fully_dynamic_property_and_builtin_api) {
    auto *s = FakeluaNewState();

    CompileConfig config;
    CompileFile(s, "./native/test_native_on_msg.lua", config);

    // ── 1. 模拟登录：Call("on_msg", "on_login", 1001, "Alice") ────────────────
    CVar login_ret;
    Call(s, JIT_TCC, "on_msg", login_ret, "on_login", 1001, "Alice");
    int64_t initial_hp = inter::FakeluaToNative<int64_t>(s, login_ret);
    EXPECT_EQ(initial_hp, 100);

    // 校验 C++ 全局管理器中的 NativeObject 实例属性
    NativeObject *alice = NativeObjectManager::Instance().Get("player", 1001);
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
    auto *s = FakeluaNewState();

    CompileConfig config;
    CompileFile(s, "./native/test_native_nested.lua", config);

    // ── 1. 执行 test_nested()，在 Lua 中创建 player 与 bag 并绑定嵌套 ────────
    CVar ret1;
    Call(s, JIT_TCC, "test_nested", ret1);
    int64_t sum = inter::FakeluaToNative<int64_t>(s, ret1);
    EXPECT_EQ(sum, 1049);// 999 + 50 = 1049

    // 校验 C++ 全局管理器与嵌套对象属性
    NativeObject *player = NativeObjectManager::Instance().Get("player", 2001);
    ASSERT_NE(player, nullptr);
    NativeObject *bag = player->GetObject("bag");
    ASSERT_NE(bag, nullptr);
    EXPECT_EQ(bag->GetInt("gold"), 999);
    EXPECT_EQ(bag->GetInt("capacity"), 50);

    // ── 2. 跨帧 Reset 内存 ─────────────────────────────────────────────────
    inter::Reset(s);

    // ── 3. 再次在 Lua 中通过 get_native_obj 获取 player 并读取 player.bag.gold ─
    CVar ret2;
    Call(s, JIT_TCC, "test_nested_fetch", ret2);
    int64_t remaining_gold = inter::FakeluaToNative<int64_t>(s, ret2);
    EXPECT_EQ(remaining_gold, 899);// 999 - 100 = 899

    // 校验 C++ 侧底层嵌套数据改变
    EXPECT_EQ(bag->GetInt("gold"), 899);

    NativeObjectManager::Instance().Clear();
    FakeluaDeleteState(s);
}

TEST(test_native, test_group_arena_batch_destroy) {
    auto *s = FakeluaNewState();

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
    EXPECT_EQ(destroyed_count, 3);// 3 个对象全部一口气清理

    // 验证销毁后这 3 个对象全部被移除
    EXPECT_EQ(NativeObjectManager::Instance().Get("player", 1001), nullptr);
    EXPECT_EQ(NativeObjectManager::Instance().Get("bag", 10010), nullptr);
    EXPECT_EQ(NativeObjectManager::Instance().Get("item", 100100), nullptr);

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_var_interface_callback) {
    auto *s = FakeluaNewState();

    // 注册以 VarInterface* 向量为入参的 C++ 回调，0 感知 CVar
    RegisterNativeVarFunction(s, "cpp_process_user", 1, false, NativeVarFuncCallback([](State *state, const std::vector<VarInterface *> &args) -> VarInterface * {
                                  EXPECT_GE(args.size(), 1);
                                  VarInterface *user_info = args[0];
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

                                  auto *res = new SimpleVarImpl();
                                  res->ViSetString(name + " has score " + std::to_string(score + 100));
                                  return res;
                              }));

    CompileConfig config;
    CompileFile(s, "./native/test_native_callback.lua", config);

    std::string result;
    Call(s, JIT_TCC, "run_test", result);
    EXPECT_EQ(result, "Bob has score 150");

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_var_interface_table_result) {
    auto *s = FakeluaNewState();

    // 回调返回一张同时含字符串键与整数键的表，Lua 侧必须能按键索引到全部内容。
    RegisterNativeVarFunction(s, "cpp_make_table", 0, false, NativeVarFuncCallback([](State *state, const std::vector<VarInterface *> &args) -> VarInterface * {
                                  auto make_str = [](const std::string &v) {
                                      auto *p = new SimpleVarImpl();
                                      p->ViSetString(v);
                                      return p;
                                  };
                                  auto make_int = [](int64_t v) {
                                      auto *p = new SimpleVarImpl();
                                      p->ViSetInt(v);
                                      return p;
                                  };

                                  std::vector<std::pair<VarInterface *, VarInterface *>> kv;
                                  kv.emplace_back(make_str("name"), make_str("Alice"));
                                  kv.emplace_back(make_str("level"), make_int(7));
                                  kv.emplace_back(make_int(1), make_int(10));
                                  kv.emplace_back(make_int(2), make_int(20));
                                  kv.emplace_back(make_int(3), make_int(30));

                                  auto *res = new SimpleVarImpl();
                                  res->ViSetTable(kv);
                                  return res;
                              }));

    CompileConfig config;
    CompileFile(s, "./native/test_native_callback.lua", config);

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        int64_t res = 0;
        Call(s, jit_type, "consume_cpp_table", res);
        EXPECT_EQ(res, 9000);
    }

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_typed_template_callback) {
    auto *s = FakeluaNewState();

    // 使用强类型 C++ 模板注册 lambda，完全零感知 CVar
    RegisterNativeFunction(s, "cpp_add_hp", false, std::function<int64_t(State *, int64_t, int64_t)>([](State *state, int64_t hp, int64_t add) -> int64_t { return hp + add; }));

    CompileConfig config;
    CompileFile(s, "./native/test_native_callback.lua", config);

    int64_t res = 0;
    Call(s, JIT_TCC, "calc", res);
    EXPECT_EQ(res, 150);

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_object_methods) {
    auto *s = FakeluaNewState();

    int64_t gid = NativeObjectManager::Instance().CreateGroup(500);
    auto *player = NativeObjectManager::Instance().Create(gid, "player", 1);

    // 1. 注册 C++ 成员回调 take_damage
    player->RegisterMethod("take_damage", [](NativeObject *self, State *state, CVar *args, int n) -> CVar {
        EXPECT_GE(n, 1);
        int64_t dmg = inter::FakeluaToNative<int64_t>(state, inter::GetNativeArg(state, args, n, 0));
        self->SetInt("hp", self->GetInt("hp") - dmg);
        return inter::NativeToFakeluaNil(state);
    });

    // 2. 注册 C++ 成员回调 is_alive
    player->RegisterMethod("is_alive", [](NativeObject *self, State *state, CVar *args, int n) -> CVar { return inter::NativeToFakeluaBool(state, self->GetInt("hp") > 0); });

    EXPECT_TRUE(player->HasMethod("take_damage"));
    EXPECT_TRUE(player->HasMethod("is_alive"));

    CompileConfig config;
    CompileFile(s, "./native/test_native_method.lua", config);

    // 3. 执行 Lua 侧测试逻辑 (使用 GCC 与 TCC 确保全面覆盖)
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        int64_t result = 0;
        Call(s, jit_type, "test_methods", result, gid);
        EXPECT_EQ(result, 1110);
    }

    // 校验 NativeObject 底层数据
    EXPECT_EQ(player->GetInt("hp"), 40);

    // 4. 取消注册测试
    player->UnregisterMethod("is_alive");
    EXPECT_FALSE(player->HasMethod("is_alive"));

    NativeObjectManager::Instance().DestroyGroup(gid);
    FakeluaDeleteState(s);
}

TEST(test_native, test_native_manager_operations) {
    // 测试 NativeObjectManager 的 DestroySingle 和 Create 返回已有对象
    int64_t gid = NativeObjectManager::Instance().CreateGroup(7001);

    auto *obj1 = NativeObjectManager::Instance().Create(gid, "unit", 1001);
    obj1->SetInt("level", 5);

    // Create 再次调用相同 (type, id) 应返回相同对象
    auto *obj1_again = NativeObjectManager::Instance().Create(gid, "unit", 1001);
    EXPECT_EQ(obj1, obj1_again);
    EXPECT_EQ(obj1_again->GetInt("level"), 5);

    auto *obj2 = NativeObjectManager::Instance().Create(gid, "unit", 1002);
    obj2->SetString("name", "warrior");

    // DestroySingle 只销毁一个
    bool destroyed = NativeObjectManager::Instance().DestroySingle("unit", 1001);
    EXPECT_TRUE(destroyed);
    EXPECT_EQ(NativeObjectManager::Instance().Get("unit", 1001), nullptr);
    EXPECT_NE(NativeObjectManager::Instance().Get("unit", 1002), nullptr);

    // 再次销毁不存在的对象
    bool again = NativeObjectManager::Instance().DestroySingle("unit", 1001);
    EXPECT_FALSE(again);

    NativeObjectManager::Instance().DestroyGroup(gid);
}

TEST(test_native, test_native_obj_advanced_fields) {
    // 测试 Del, Clear, SetNil, GetAsCVar, ForEach, SetId/SetGroupId
    int64_t gid = NativeObjectManager::Instance().CreateGroup(7002);
    auto *obj = NativeObjectManager::Instance().Create(gid, "hero", 2001);

    // SetId / GetId
    obj->SetId(9999);
    EXPECT_EQ(obj->GetId(), 9999);
    obj->SetId(2001);// restore

    // SetGroupId / GetGroupId
    obj->SetGroupId(7002);
    EXPECT_EQ(obj->GetGroupId(), 7002);

    // SetInt / GetInt with cross-type access
    obj->SetFloat("speed", 3.14);
    // GetInt on float field should cast
    int64_t speed_int = obj->GetInt("speed", 0);
    EXPECT_EQ(speed_int, 3);// floor of 3.14

    // GetFloat on int field should cast
    obj->SetInt("level", 5);
    double level_f = obj->GetFloat("level", 0.0);
    EXPECT_DOUBLE_EQ(level_f, 5.0);

    // GetBool on missing field returns default
    bool b = obj->GetBool("nonexist", true);
    EXPECT_TRUE(b);

    // GetString on missing field returns default
    std::string s = obj->GetString("nonexist", "default");
    EXPECT_EQ(s, "default");

    // GetObject on missing field returns nullptr
    NativeObject *nested = obj->GetObject("nonexist");
    EXPECT_EQ(nested, nullptr);

    // Has/Del
    obj->SetBool("alive", true);
    EXPECT_TRUE(obj->Has("alive"));
    obj->Del("alive");
    EXPECT_FALSE(obj->Has("alive"));

    // SetNil removes field
    obj->SetString("tag", "hero");
    EXPECT_TRUE(obj->Has("tag"));
    obj->SetNil("tag");
    EXPECT_FALSE(obj->Has("tag"));

    // SetInt / SetString verify round-trip via Has/GetInt
    obj->SetString("item", "sword");
    EXPECT_EQ(obj->GetString("item"), "sword");
    obj->SetInt("qty", 5);
    EXPECT_EQ(obj->GetInt("qty"), 5);

    // ForEach
    obj->SetInt("a", 1);
    obj->SetInt("b", 2);
    int field_count = 0;
    obj->ForEach([&](std::string_view key, NativeObject::FieldKind kind) {
        field_count++;
    });
    EXPECT_GE(field_count, 2);

    // Clear removes all fields and methods
    obj->Clear();
    EXPECT_EQ(obj->Size(), 0);

    NativeObjectManager::Instance().DestroyGroup(7002);
}

TEST(test_native, test_native_lua_manager_ops) {
    auto *s = FakeluaNewState();

    CompileConfig config;
    CompileFile(s, "./native/test_native_manager.lua", config);

    // test_destroy_single
    CVar ret1;
    Call(s, JIT_TCC, "test_destroy_single", ret1);
    EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret1), 5000);

    inter::Reset(s);

    // test_new_group_auto_id
    CVar ret2;
    Call(s, JIT_TCC, "test_new_group_auto_id", ret2);
    EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret2), 5000);

    inter::Reset(s);

    // test_create_existing
    CVar ret3;
    Call(s, JIT_TCC, "test_create_existing", ret3);
    EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret3), 5000);

    inter::Reset(s);

    // test_get_nil_args
    CVar ret4;
    Call(s, JIT_TCC, "test_get_nil_args", ret4);
    EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret4), 5000);

    NativeObjectManager::Instance().Clear();
    FakeluaDeleteState(s);
}

TEST(test_native, test_native_lua_obj_ops) {
    auto *s = FakeluaNewState();

    CompileConfig config;
    CompileFile(s, "./native/test_native_obj_ops.lua", config);

    struct TestCase {
        const char *func_name;
    };
    for (auto &tc : (std::vector<TestCase>{
                 {"test_del_field"},
                 {"test_float_field"},
                 {"test_bool_field"},
                 {"test_string_field"},
                 {"test_object_field"},
                 {"test_pairs_on_native_obj"},
                 {"test_int_bool_not_equal_cross"},
             })) {
        inter::Reset(s);
        NativeObjectManager::Instance().Clear();
        CVar ret;
        Call(s, JIT_TCC, tc.func_name, ret);
        EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret), 5000) << "FAILED: " << tc.func_name;
    }

    NativeObjectManager::Instance().Clear();
    FakeluaDeleteState(s);
}

TEST(test_native, test_native_obj_wrap) {
    auto *s = FakeluaNewState();

    CompileConfig config;
    CompileFile(s, "./native/test_native_obj_wrap.lua", config);

    struct TestCase {
        const char *func_name;
    };
    for (auto &tc : (std::vector<TestCase>{
                 {"test_wrap_object_field"},
                 {"test_wrap_empty_spec_keys"},
                 {"test_wrap_set_from_cvar"},
                 {"test_wrap_get_as_cvar"},
             })) {
        inter::Reset(s);
        NativeObjectManager::Instance().Clear();
        CVar ret;
        Call(s, JIT_TCC, tc.func_name, ret);
        EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret), 5000) << "FAILED: " << tc.func_name;
    }

    NativeObjectManager::Instance().Clear();
    FakeluaDeleteState(s);
}

TEST(test_native, test_native_manager_clear) {
    // Test NativeObjectManager::Clear() - destroys all objects
    int64_t gid1 = NativeObjectManager::Instance().CreateGroup(8001);
    int64_t gid2 = NativeObjectManager::Instance().CreateGroup(8002);

    auto *obj1 = NativeObjectManager::Instance().Create(gid1, "unit", 1);
    auto *obj2 = NativeObjectManager::Instance().Create(gid2, "unit", 2);
    obj1->SetInt("x", 10);
    obj2->SetInt("y", 20);

    EXPECT_NE(NativeObjectManager::Instance().Get("unit", 1), nullptr);
    EXPECT_NE(NativeObjectManager::Instance().Get("unit", 2), nullptr);

    // Clear all
    NativeObjectManager::Instance().Clear();

    // After Clear, all objects should be gone
    EXPECT_EQ(NativeObjectManager::Instance().Get("unit", 1), nullptr);
    EXPECT_EQ(NativeObjectManager::Instance().Get("unit", 2), nullptr);
}

TEST(test_native, test_native_create_error) {
    // Test NativeObjectManager::Create with group_id == 0 (error path)
    EXPECT_THROW(NativeObjectManager::Instance().Create(0, "test", 1), FakeluaException);
}

TEST(test_native, test_native_manager_destroy_empty) {
    // DestroyGroup on non-existent group returns 0
    size_t count = NativeObjectManager::Instance().DestroyGroup(99999);
    EXPECT_EQ(count, 0);

    // DestroySingle on non-existent object returns false
    bool result = NativeObjectManager::Instance().DestroySingle("nonexist", 99999);
    EXPECT_FALSE(result);
}

