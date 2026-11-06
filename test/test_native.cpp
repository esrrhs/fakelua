#include "fakelua.h"
#include "state/state.h"
#include "var/var_table.h"
#include "test_jit.h"
#include "var/var_type.h"
#include "gtest/gtest.h"
#include <string>
#include <unordered_map>

using namespace fakelua;

TEST(test_native, test_basic_kv) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *obj = mgr.Create(gid, "player", 1);
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

    mgr.DestroyGroup(gid);

    FakeluaDeleteState(s);
}

TEST(test_native, access_after_destroy_group) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *obj = mgr.Create(gid, "ghost", 1);
    obj->SetInt("hp", 9);
    mgr.DestroyGroup(gid);
    EXPECT_FALSE(obj->Alive());
    EXPECT_EQ(obj->GetInt("hp"), 0);
    EXPECT_FALSE(obj->Has("hp"));
    EXPECT_FALSE(obj->GetBool("alive"));
    EXPECT_DOUBLE_EQ(obj->GetFloat("speed"), 0.0);
    EXPECT_TRUE(obj->GetString("name").empty());
    EXPECT_EQ(obj->GetObject("bag"), nullptr);
    obj->SetFloat("x", 1.0);
    obj->SetBool("y", true);
    obj->SetString("z", "nope");
    obj->Del("hp");
    obj->Clear();
    obj->RegisterMethod("noop", NativeMethod{});
    EXPECT_FALSE(obj->HasMethod("noop"));
    mgr.Clear();

    FakeluaDeleteState(s);
}

TEST(test_native, test_nested_object) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *player = mgr.Create(gid, "player", 1);
    auto *bag = mgr.Create(gid, "bag", 2);

    bag->SetInt("gold", 999);
    player->SetObject("bag", bag);

    EXPECT_EQ(player->GetObject("bag"), bag);
    EXPECT_EQ(player->GetObject("bag")->GetInt("gold"), 999);

    mgr.DestroyGroup(gid);

    FakeluaDeleteState(s);
}


TEST(test_native, test_deep_nesting_wrap_is_light) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();

    NativeObject *root = mgr.Create(gid, "node", 1);
    NativeObject *cur = root;
    constexpr int kDepth = 64;
    for (int i = 1; i <= kDepth; ++i) {
        NativeObject *child = mgr.Create(gid, "node", i + 1);
        child->SetInt("depth", i);
        cur->SetObject("child", child);
        cur = child;
    }

    // Bushy fan-out on root would explode under recursive Wrap+RefreshSpecKeys.
    for (int i = 0; i < 32; ++i) {
        NativeObject *leaf = mgr.Create(gid, "leaf", 1000 + i);
        leaf->SetInt("i", i);
        root->SetObject(std::string("leaf_") + std::to_string(i), leaf);
    }

    inter::Reset(s);
    const size_t before = s->GetValueAllocator().Size();
    CVar wrapped = root->Wrap(s);
    const size_t after_wrap = s->GetValueAllocator().Size();
    // Light shell only: VarTable + NativeObjectSpec. Must not allocate O(depth * fanout) snapshots.
    EXPECT_LT(after_wrap - before, 4096u);
    EXPECT_EQ(wrapped.type_, static_cast<int>(VarType::Table));
    ASSERT_NE(wrapped.data_.t, nullptr);
    EXPECT_EQ(wrapped.data_.t->spec_count, 0u);// snapshot still lazy after Wrap

    NativeObject *unwrapped = NativeObject::Unwrap(wrapped);
    ASSERT_EQ(unwrapped, root);
    NativeObject *child = root->GetObject("child");
    ASSERT_NE(child, nullptr);
    EXPECT_EQ(child->GetInt("depth"), 1);

    // Nested object's own Wrap is also light (no eager snapshot / no recursive tree).
    const size_t before_child = s->GetValueAllocator().Size();
    CVar child_cv = child->Wrap(s);
    const size_t after_child = s->GetValueAllocator().Size();
    EXPECT_LT(after_child - before_child, 4096u);
    ASSERT_EQ(child_cv.type_, static_cast<int>(VarType::Table));
    ASSERT_NE(child_cv.data_.t, nullptr);
    EXPECT_EQ(child_cv.data_.t->spec_count, 0u);
    EXPECT_EQ(NativeObject::Unwrap(child_cv), child);

    mgr.DestroyGroup(gid);
    FakeluaDeleteState(s);
}

TEST(test_native, test_lua_integration_get_set) {
    for (auto jit_type: AllJitTypes()) {
        auto *s = FakeluaNewState();
        auto &mgr = GetNativeObjectManager(s);

        int64_t gid = mgr.CreateGroup();
        static NativeObject *global_player = nullptr;
        global_player = mgr.Create(gid, "player", 100);
        global_player->SetInt("hp", 100);

        RegisterNativeFunction(s, "get_player", 0, false, [](State *state, CVar *args, int n) -> CVar { return inter::NativeToFakelua(state, global_player); });

        CompileConfig config;
        CompileFile(s, "./native/test_native_get_set.lua", config);

        CVar ret;
        Call(s, jit_type, "test_get_set", ret);

        EXPECT_EQ(global_player->GetInt("hp"), 150);

        mgr.DestroyGroup(gid);
        global_player = nullptr;
        FakeluaDeleteState(s);
    }
}

TEST(test_native, test_fully_dynamic_property_and_builtin_api) {
    for (auto jit_type: AllJitTypes()) {
        auto *s = FakeluaNewState();
        auto &mgr = GetNativeObjectManager(s);

        CompileConfig config;
        CompileFile(s, "./native/test_native_on_msg.lua", config);

        // 1. 模拟登录：Call("on_msg", "on_login", 1001, "Alice")
        CVar login_ret;
        Call(s, jit_type, "on_msg", login_ret, "on_login", 1001, "Alice");
        int64_t initial_hp = inter::FakeluaToNative<int64_t>(s, login_ret);
        EXPECT_EQ(initial_hp, 100);

        // 校验 C++ 全局管理器中的 NativeObject 实例属性
        NativeObject *alice = mgr.Get("player", 1001);
        ASSERT_NE(alice, nullptr);
        EXPECT_EQ(alice->GetInt("hp"), 100);
        EXPECT_EQ(alice->GetInt("mp"), 200);
        EXPECT_EQ(alice->GetString("name"), "Alice");

        // 2. 模拟跨帧 reset（Arena 内存重置）
        inter::Reset(s);

        // 3. 模拟对话：Call("on_msg", "on_talk", 1001, "Hello fakelua!")
        CVar talk_ret;
        Call(s, jit_type, "on_msg", talk_ret, "on_talk", 1001, "Hello fakelua!");
        std::string talk_res = inter::FakeluaToNative<std::string>(s, talk_ret);
        EXPECT_EQ(talk_res, "Alice (80hp): Hello fakelua!");

        // 校验 C++ 侧持久数据：hp 扣减为 80，动态字段 last_talk 成功设置
        EXPECT_EQ(alice->GetInt("hp"), 80);
        EXPECT_EQ(alice->GetString("last_talk"), "Hello fakelua!");

        // 清理全局 NativeObjectManager
        mgr.Clear();
        FakeluaDeleteState(s);
    }
}

TEST(test_native, test_lua_nested_object) {
    for (auto jit_type: AllJitTypes()) {
        auto *s = FakeluaNewState();
        auto &mgr = GetNativeObjectManager(s);

        CompileConfig config;
        CompileFile(s, "./native/test_native_nested.lua", config);

        // 1. 执行 test_nested()，在 Lua 中创建 player 与 bag 并绑定嵌套
        CVar ret1;
        Call(s, jit_type, "test_nested", ret1);
        int64_t sum = inter::FakeluaToNative<int64_t>(s, ret1);
        EXPECT_EQ(sum, 1049);// 999 + 50 = 1049

        // 校验 C++ 全局管理器与嵌套对象属性
        NativeObject *player = mgr.Get("player", 2001);
        ASSERT_NE(player, nullptr);
        NativeObject *bag = player->GetObject("bag");
        ASSERT_NE(bag, nullptr);
        EXPECT_EQ(bag->GetInt("gold"), 999);
        EXPECT_EQ(bag->GetInt("capacity"), 50);

        // 2. 跨帧 Reset 内存
        inter::Reset(s);

        // 3. 再次在 Lua 中通过 get_native_obj 获取 player 并读取 player.bag.gold
        CVar ret2;
        Call(s, jit_type, "test_nested_fetch", ret2);
        int64_t remaining_gold = inter::FakeluaToNative<int64_t>(s, ret2);
        EXPECT_EQ(remaining_gold, 899);// 999 - 100 = 899

        // 校验 C++ 侧底层嵌套数据改变
        EXPECT_EQ(bag->GetInt("gold"), 899);

        mgr.Clear();
        FakeluaDeleteState(s);
    }
}

TEST(test_native, test_group_arena_batch_destroy) {
    for (auto jit_type: AllJitTypes()) {
        auto *s = FakeluaNewState();
        auto &mgr = GetNativeObjectManager(s);

        CompileConfig config;
        CompileFile(s, "./native/test_native_group.lua", config);

        // 1. 在 Lua 中创建玩家 1001 以及归属于该 group 的 bag 和 item 对象
        CVar ret1;
        Call(s, jit_type, "test_group_create", ret1, 1001);
        int64_t gid = inter::FakeluaToNative<int64_t>(s, ret1);
        EXPECT_GT(gid, 0);

        // 验证对象存在于管理器中
        auto *item = mgr.Get("item", 100100);
        ASSERT_NE(item, nullptr);
        EXPECT_EQ(item->GetString("name"), "Excalibur");
        EXPECT_NE(mgr.Get("player", 1001), nullptr);
        EXPECT_NE(mgr.Get("bag", 10010), nullptr);

        // 2. 一口气批处理销毁该组下的所有 NativeObject
        CVar ret2;
        Call(s, jit_type, "test_group_destroy", ret2, gid);
        int64_t destroyed_count = inter::FakeluaToNative<int64_t>(s, ret2);
        EXPECT_EQ(destroyed_count, 3);// 3 个对象全部一口气清理

        // 验证销毁后这 3 个对象全部被移除
        EXPECT_EQ(mgr.Get("player", 1001), nullptr);
        EXPECT_EQ(mgr.Get("bag", 10010), nullptr);
        EXPECT_EQ(mgr.Get("item", 100100), nullptr);

        FakeluaDeleteState(s);
    }
}

TEST(test_native, test_native_var_interface_callback) {
    for (auto jit_type: AllJitTypes()) {
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
        Call(s, jit_type, "run_test", result);
        EXPECT_EQ(result, "Bob has score 150");

        FakeluaDeleteState(s);
    }
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

    for (auto jit_type: AllJitTypes()) {
        int64_t res = 0;
        Call(s, jit_type, "consume_cpp_table", res);
        EXPECT_EQ(res, 9000);
    }

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_typed_template_callback) {
    for (auto jit_type: AllJitTypes()) {
        auto *s = FakeluaNewState();

        // 使用强类型 C++ 模板注册 lambda，完全零感知 CVar
        RegisterNativeFunction(s, "cpp_add_hp", false, std::function<int64_t(State *, int64_t, int64_t)>([](State *state, int64_t hp, int64_t add) -> int64_t { return hp + add; }));

        CompileConfig config;
        CompileFile(s, "./native/test_native_callback.lua", config);

        int64_t res = 0;
        Call(s, jit_type, "calc", res);
        EXPECT_EQ(res, 150);

        FakeluaDeleteState(s);
    }
}

// 宿主写原生回调时读参数、回传标量都不该伸进 inter::（那是解释器/JIT 的实现细节）。
TEST(test_native, host_scalar_arg_helpers) {
    auto *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    RegisterNativeFunction(s, "host_join", 0, true, [](State *st, CVar *args, int n) -> CVar {
        const std::string name = GetNativeStringArg(st, args, n, 0, "<none>");
        const int64_t id = GetNativeIntArg(st, args, n, 1, -1);
        // 缺省的第 3、4 个参数走默认值，不当成错误
        const bool flag = GetNativeBoolArg(st, args, n, 2, true);
        const double ratio = GetNativeFloatArg(st, args, n, 3, 0.5);
        return NativeString(st, name + ":" + std::to_string(id) + (flag ? ":T" : ":F") + ":" + std::to_string(ratio));
    });

    RegisterNativeFunction(s, "host_scalar_roundtrip", 0, true, [](State *st, CVar *args, int n) -> CVar {
        // 数字/布尔参数按 Lua 语义宽容读取：bool 当 0/1
        const int64_t from_bool = GetNativeIntArg(st, args, n, 0, 42);
        const bool from_int = GetNativeBoolArg(st, args, n, 1, false);
        if (from_int) return NativeInt(st, from_bool);
        return NativeBool(st, from_bool == 0);
    });

    CompileConfig cfg;
    ASSERT_NO_THROW(CompileString(s, R"(package "HostHelpers"
function defaults()
    return host_join("p1", 7)
end
function omitted()
    return host_join()
end
function bool_as_int()
    return host_scalar_roundtrip(true, 1)
end
function zero_flag()
    -- Lua 语义下 0 是真值，所以 from_int 为 true，返回 from_bool (9)
    if host_scalar_roundtrip(9, 0) == 9 then return 1 else return 0 end
end
function false_flag()
    -- false 是假值，from_int 为 false，返回 NativeBool(from_bool == 0) -> true
    if host_scalar_roundtrip(0, false) then return 1 else return 0 end
end
)""", cfg));

    for (auto jit_type: AllJitTypes()) {
        std::string out;
        Call(s, jit_type, "HostHelpers.defaults", out);
        EXPECT_EQ(out, "p1:7:T:0.500000") << JitTypeName(jit_type);

        Call(s, jit_type, "HostHelpers.omitted", out);
        EXPECT_EQ(out, "<none>:-1:T:0.500000") << JitTypeName(jit_type);

        int64_t ret = 0;
        Call(s, jit_type, "HostHelpers.bool_as_int", ret);
        EXPECT_EQ(ret, 1) << JitTypeName(jit_type);

        Call(s, jit_type, "HostHelpers.zero_flag", ret);
        EXPECT_EQ(ret, 1) << JitTypeName(jit_type);

        Call(s, jit_type, "HostHelpers.false_flag", ret);
        EXPECT_EQ(ret, 1) << JitTypeName(jit_type);
    }

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_object_methods) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);

    int64_t gid = mgr.CreateGroup();
    auto *player = mgr.Create(gid, "player", 1);

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
    for (auto jit_type: AllJitTypes()) {
        int64_t result = 0;
        Call(s, jit_type, "test_methods", result, gid);
        EXPECT_EQ(result, 1110);
    }

    // 校验 NativeObject 底层数据
    EXPECT_EQ(player->GetInt("hp"), 40);

    // 4. 取消注册测试
    player->UnregisterMethod("is_alive");
    EXPECT_FALSE(player->HasMethod("is_alive"));

    mgr.DestroyGroup(gid);
    FakeluaDeleteState(s);
}

TEST(test_native, test_native_manager_operations) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    // 测试 NativeObjectManager 的 DestroySingle 和 Create 返回已有对象
    int64_t gid = mgr.CreateGroup();

    auto *obj1 = mgr.Create(gid, "unit", 1001);
    obj1->SetInt("level", 5);

    // Create 再次调用相同 (type, id) 应返回相同对象
    auto *obj1_again = mgr.Create(gid, "unit", 1001);
    EXPECT_EQ(obj1, obj1_again);
    EXPECT_EQ(obj1_again->GetInt("level"), 5);

    auto *obj2 = mgr.Create(gid, "unit", 1002);
    obj2->SetString("name", "warrior");

    // DestroySingle 只销毁一个
    bool destroyed = mgr.DestroySingle("unit", 1001);
    EXPECT_TRUE(destroyed);
    EXPECT_EQ(mgr.Get("unit", 1001), nullptr);
    EXPECT_NE(mgr.Get("unit", 1002), nullptr);

    // 再次销毁不存在的对象
    bool again = mgr.DestroySingle("unit", 1001);
    EXPECT_FALSE(again);

    mgr.DestroyGroup(gid);

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_obj_advanced_fields) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    // 测试 Del, Clear, SetNil, GetAsCVar, ForEach, SetId/SetGroupId
    int64_t gid = mgr.CreateGroup();
    auto *obj = mgr.Create(gid, "hero", 2001);

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
    std::string missing_str = obj->GetString("nonexist", "default");
    EXPECT_EQ(missing_str, "default");

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
    obj->ForEach([&](std::string_view key, NativeObject::FieldKind kind) { field_count++; });
    EXPECT_GE(field_count, 2);

    // Clear removes all fields and methods
    obj->Clear();
    EXPECT_EQ(obj->Size(), 0);

    mgr.DestroyGroup(7002);

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_lua_manager_ops) {
    for (auto jit_type: AllJitTypes()) {
        auto *s = FakeluaNewState();
        auto &mgr = GetNativeObjectManager(s);

        CompileConfig config;
        CompileFile(s, "./native/test_native_manager.lua", config);

        // test_destroy_single
        CVar ret1;
        Call(s, jit_type, "test_destroy_single", ret1);
        EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret1), 5000);

        inter::Reset(s);

        // test_new_group_auto_id
        CVar ret2;
        Call(s, jit_type, "test_new_group_auto_id", ret2);
        EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret2), 5000);

        inter::Reset(s);

        // test_create_existing
        CVar ret3;
        Call(s, jit_type, "test_create_existing", ret3);
        EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret3), 5000);

        inter::Reset(s);

        // test_get_nil_args
        CVar ret4;
        Call(s, jit_type, "test_get_nil_args", ret4);
        EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret4), 5000);

        inter::Reset(s);

        CVar ret5;
        Call(s, jit_type, "test_access_after_destroy", ret5);
        EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret5), 5000);

        mgr.Clear();
        FakeluaDeleteState(s);
    }
}

TEST(test_native, test_native_lua_obj_ops) {
    for (auto jit_type: AllJitTypes()) {
        auto *s = FakeluaNewState();
        auto &mgr = GetNativeObjectManager(s);

        CompileConfig config;
        CompileFile(s, "./native/test_native_obj_ops.lua", config);

        struct TestCase {
            const char *func_name;
        };

        for (auto &tc: (std::vector<TestCase>{
                     {"test_del_field"},
                     {"test_float_field"},
                     {"test_bool_field"},
                     {"test_string_field"},
                     {"test_object_field"},
                     {"test_pairs_on_native_obj"},
                     {"test_pairs_after_incremental_spec_set"},
                     {"test_deep_nesting_lazy_wrap"},
                     {"test_pairs_after_lazy_writes"},
                     {"test_next_on_native_obj"},
                     {"test_int_bool_not_equal_cross"},
             })) {
            inter::Reset(s);
            mgr.Clear();
            CVar ret;
            Call(s, jit_type, tc.func_name, ret);
            EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret), 5000) << "FAILED: " << tc.func_name;
        }

        mgr.Clear();
        FakeluaDeleteState(s);
    }
}

TEST(test_native, test_native_obj_wrap) {
    for (auto jit_type: AllJitTypes()) {
        auto *s = FakeluaNewState();
        auto &mgr = GetNativeObjectManager(s);

        CompileConfig config;
        CompileFile(s, "./native/test_native_obj_wrap.lua", config);

        struct TestCase {
            const char *func_name;
        };

        for (auto &tc: (std::vector<TestCase>{
                     {"test_wrap_object_field"},
                     {"test_wrap_empty_spec_keys"},
                     {"test_wrap_set_from_cvar"},
                     {"test_wrap_get_as_cvar"},
             })) {
            inter::Reset(s);
            mgr.Clear();
            CVar ret;
            Call(s, jit_type, tc.func_name, ret);
            EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret), 5000) << "FAILED: " << tc.func_name;
        }

        mgr.Clear();
        FakeluaDeleteState(s);
    }
}

TEST(test_native, test_native_manager_clear) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    // Test NativeObjectManager::Clear() - destroys all objects
    int64_t gid1 = mgr.CreateGroup();
    int64_t gid2 = mgr.CreateGroup();

    auto *obj1 = mgr.Create(gid1, "unit", 1);
    auto *obj2 = mgr.Create(gid2, "unit", 2);
    obj1->SetInt("x", 10);
    obj2->SetInt("y", 20);

    EXPECT_NE(mgr.Get("unit", 1), nullptr);
    EXPECT_NE(mgr.Get("unit", 2), nullptr);

    // Clear all
    mgr.Clear();

    // After Clear, all objects should be gone
    EXPECT_EQ(mgr.Get("unit", 1), nullptr);
    EXPECT_EQ(mgr.Get("unit", 2), nullptr);

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_create_error) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    // Test NativeObjectManager::Create with group_id == 0 (error path)
    EXPECT_THROW(mgr.Create(0, "test", 1), FakeluaException);

    FakeluaDeleteState(s);
}

TEST(test_native, test_native_manager_destroy_empty) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    // DestroyGroup on non-existent group returns 0
    size_t count = mgr.DestroyGroup(99999);
    EXPECT_EQ(count, 0);

    // DestroySingle on non-existent object returns false
    bool result = mgr.DestroySingle("nonexist", 99999);
    EXPECT_FALSE(result);

    FakeluaDeleteState(s);
}

TEST(test_native, register_native_var_and_func_addr) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    RegisterNativeVarFunction(s, "cov_make_tbl", 0, false, [](State *state, const std::vector<VarInterface *> &) -> VarInterface * {
        auto *t = new SimpleVarImpl();
        std::vector<std::pair<VarInterface *, VarInterface *>> kvs;
        auto *k = new SimpleVarImpl();
        k->ViSetString("a");
        auto *v = new SimpleVarImpl();
        v->ViSetInt(1);
        kvs.emplace_back(k, v);
        t->ViSetTable(kvs);
        delete k;
        delete v;
        return t;
    });
    // null callback path
    RegisterNativeVarFunction(s, "cov_nil_cb", 0, false, NativeVarFuncCallback{});

    CompileConfig config;
    CompileString(s, R"(
function test_native_var()
    local t = cov_make_tbl()
    if t.a ~= 1 then return 1 end
    local n = cov_nil_cb()
    if n ~= nil then return 2 end
    return 0
end
)",
                  config);
    double res = 0;
    Call(s, JIT_GCC, "test_native_var", res);
    EXPECT_EQ(res, 0.0);

    int argc = 0;
    bool is_va = false;
    void *addr = inter::GetFuncAddr(s, JIT_GCC, "test_native_var", argc, is_va);
    EXPECT_NE(addr, nullptr);
    EXPECT_EQ(inter::GetFuncAddr(s, JIT_GCC, "no_such_fn_xyz", argc, is_va), nullptr);

    SetDebugLogLevel(s, 1);
    SetDebugLogLevel(s, LogLevel::Warn);
    FakeluaDeleteState(s);
}

// ==================== 对象级变更版本号 ====================

TEST(test_native_version, all_write_paths_bump) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *obj = mgr.Create(gid, "vobj", 1);
    auto *child = mgr.Create(gid, "vchild", 2);

    const auto expect_bump = [&](const char *what, const std::function<void()> &fn) {
        const uint64_t before = obj->GetVersion();
        fn();
        EXPECT_GT(obj->GetVersion(), before) << "write path not counted: " << what;
    };

    expect_bump("SetInt", [&] { obj->SetInt("i", 1); });
    expect_bump("SetFloat", [&] { obj->SetFloat("f", 1.5); });
    expect_bump("SetBool", [&] { obj->SetBool("b", true); });
    expect_bump("SetString", [&] { obj->SetString("s", "x"); });
    expect_bump("SetObject", [&] { obj->SetObject("c", child); });
    expect_bump("SetFromCVar", [&] { NativeObjectVersionTestAccess::SetFromCVar(obj, "cv", inter::NativeToFakeluaLonglong(s, 7)); });
    expect_bump("SetNil(existing)", [&] { obj->SetNil("i"); });
    expect_bump("Del(existing)", [&] { obj->Del("f"); });
    expect_bump("Del(missing)", [&] { obj->Del("missing"); });
    expect_bump("RegisterMethod", [&] { obj->RegisterMethod("m", NativeMethod{}); });
    expect_bump("UnregisterMethod", [&] { obj->UnregisterMethod("m"); });
    expect_bump("SetId", [&] { obj->SetId(11); });
    expect_bump("SetGroupId", [&] { obj->SetGroupId(gid); });
    expect_bump("SetFinalizer", [&] { obj->SetFinalizer([](NativeObject *) {}); });
    expect_bump("MarkModified", [&] { obj->MarkModified(); });
    expect_bump("Clear", [&] { obj->Clear(); });

    // 版本号不是可见字段，不改变既有数据格式
    EXPECT_FALSE(obj->Has("version"));
    EXPECT_FALSE(obj->Has("__version__"));

    // 本用例改过 id，manager 的 (type,id) 索引与对象当前 id 不一致；
    // 用 Clear() 统一销毁，避免 DestroyGroup 按新 id 擦除索引造成重复释放。
    mgr.Clear();
    FakeluaDeleteState(s);
}

TEST(test_native_version, read_paths_do_not_bump) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *obj = mgr.Create(gid, "vobj", 1);
    auto *child = mgr.Create(gid, "vchild", 2);
    child->SetInt("gold", 5);
    obj->SetInt("hp", 10);
    obj->SetFloat("speed", 2.5);
    obj->SetBool("alive", true);
    obj->SetString("name", "hero");
    obj->SetObject("child", child);
    obj->RegisterMethod("m", NativeMethod{});

    const uint64_t v = obj->GetVersion();
    const uint64_t vc = child->GetVersion();

    EXPECT_EQ(obj->GetInt("hp"), 10);
    EXPECT_DOUBLE_EQ(obj->GetFloat("speed"), 2.5);
    EXPECT_TRUE(obj->GetBool("alive"));
    EXPECT_EQ(obj->GetString("name"), "hero");
    EXPECT_EQ(obj->GetObject("child"), child);
    EXPECT_EQ(obj->GetInt("missing", 42), 42);
    EXPECT_TRUE(obj->Has("hp"));
    EXPECT_EQ(obj->Size(), 5);
    EXPECT_TRUE(obj->HasMethod("m"));
    EXPECT_EQ(obj->GetId(), 1);
    EXPECT_EQ(obj->GetGroupId(), gid);
    EXPECT_NE(obj->GetTypeName(), "");
    int fields = 0;
    obj->ForEach([&](std::string_view, NativeObject::FieldKind) { ++fields; });
    EXPECT_EQ(fields, 5);

    CVar as_cvar = NativeObjectVersionTestAccess::GetAsCVar(obj, "name", s);
    EXPECT_EQ(inter::FakeluaToNative<std::string>(s, as_cvar), "hero");
    CVar wrapped = obj->Wrap(s);
    EXPECT_EQ(NativeObject::Unwrap(wrapped), obj);

    EXPECT_EQ(obj->GetVersion(), v);
    EXPECT_EQ(child->GetVersion(), vc);

    mgr.DestroyGroup(gid);
    FakeluaDeleteState(s);
}

TEST(test_native_version, nested_propagation_and_isolation) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *root = mgr.Create(gid, "root", 1);
    auto *a = mgr.Create(gid, "a", 2);
    auto *b = mgr.Create(gid, "b", 3);
    auto *leaf = mgr.Create(gid, "leaf", 4);
    root->SetObject("a", a);
    root->SetObject("b", b);
    a->SetObject("leaf", leaf);

    uint64_t vr = root->GetVersion(), va = a->GetVersion(), vb = b->GetVersion();
    const uint64_t vl = leaf->GetVersion();
    leaf->SetInt("x", 1);
    EXPECT_TRUE(root->IsChangedSince(vr));
    EXPECT_TRUE(a->IsChangedSince(va));
    EXPECT_TRUE(leaf->IsChangedSince(vl));
    EXPECT_FALSE(b->IsChangedSince(vb));

    va = a->GetVersion();
    vb = b->GetVersion();
    b->SetInt("y", 2);
    EXPECT_TRUE(root->IsChangedSince(vb));
    EXPECT_TRUE(b->IsChangedSince(vb));
    EXPECT_FALSE(a->IsChangedSince(va));

    // 共享子对象：被多个父对象引用时都能感知
    auto *p1 = mgr.Create(gid, "p1", 5);
    auto *p2 = mgr.Create(gid, "p2", 6);
    p1->SetObject("c", leaf);
    p2->SetObject("c", leaf);
    uint64_t v1 = p1->GetVersion(), v2 = p2->GetVersion();
    leaf->SetInt("x", 2);
    EXPECT_TRUE(p1->IsChangedSince(v1));
    EXPECT_TRUE(p2->IsChangedSince(v2));

    // 同一父对象的多字段引用：任一字段仍在引用时，写入仍能传播
    p1->SetObject("d", leaf);
    p1->SetNil("c");
    v1 = p1->GetVersion();
    leaf->SetInt("x", 3);
    EXPECT_TRUE(p1->IsChangedSince(v1));
    p1->SetNil("d");
    v1 = p1->GetVersion();
    leaf->SetInt("x", 4);
    EXPECT_FALSE(p1->IsChangedSince(v1));

    mgr.DestroyGroup(gid);
    FakeluaDeleteState(s);
}

TEST(test_native_version, child_create_destroy_and_clear) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *parent = mgr.Create(gid, "parent", 1);
    auto *child = mgr.Create(gid, "child", 2);

    uint64_t vp = parent->GetVersion();
    parent->SetObject("child", child);
    EXPECT_TRUE(parent->IsChangedSince(vp));// 子对象创建/挂载

    vp = parent->GetVersion();
    const uint64_t vc = child->GetVersion();
    mgr.DestroySingle("child", 2);
    EXPECT_TRUE(parent->IsChangedSince(vp));// 子对象删除
    EXPECT_FALSE(child->Alive());
    EXPECT_EQ(child->GetVersion(), vc);// 自身最后一次版本保留，单调不回退

    auto *c2 = mgr.Create(gid, "child2", 3);
    parent->SetObject("c2", c2);
    vp = parent->GetVersion();
    parent->Clear();
    EXPECT_TRUE(parent->IsChangedSince(vp));// 重置/批量清空

    mgr.DestroyGroup(gid);
    FakeluaDeleteState(s);
}

TEST(test_native_version, monotonic_and_saturation) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *obj = mgr.Create(gid, "vobj", 1);

    uint64_t prev = obj->GetVersion();
    for (int i = 0; i < 1000; ++i) {
        obj->SetInt("x", i);
        EXPECT_GT(obj->GetVersion(), prev);
        prev = obj->GetVersion();
    }

    // 饱和：计数器达到 UINT64_MAX 后不再回绕，版本号保持最大值（不再可区分新旧）。
    const uint64_t saved = NativeObjectVersionTestAccess::GetStamp();
    NativeObjectVersionTestAccess::SetStamp(UINT64_MAX);
    obj->SetInt("x", 1);
    const uint64_t sat1 = obj->GetVersion();
    obj->SetInt("x", 2);
    const uint64_t sat2 = obj->GetVersion();
    NativeObjectVersionTestAccess::SetStamp(saved);
    EXPECT_EQ(sat1, UINT64_MAX);
    EXPECT_EQ(sat2, UINT64_MAX);
    EXPECT_FALSE(obj->IsChangedSince(UINT64_MAX));

    mgr.DestroyGroup(gid);
    FakeluaDeleteState(s);
}

TEST(test_native_version, debug_self_check_detects_missed_bump) {
    NativeObject::SetVersionDebugEnabled(true);
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *root = mgr.Create(gid, "root", 1);
    auto *child = mgr.Create(gid, "child", 2);
    root->SetObject("child", child);
    child->SetInt("a", 1);
    child->RegisterMethod("m", NativeMethod{});

    EXPECT_TRUE(NativeObject::SelfCheck(root, true));
    EXPECT_TRUE(NativeObject::SelfCheck(child, false));

    // 绕过对象 API 的直接写入（模拟漏标写入点）：版本号不变，但内容哈希不一致。
    const uint64_t vc = child->GetVersion();
    NativeObjectVersionTestAccess::RawSetInt(child, "bypass", 7);
    EXPECT_EQ(child->GetVersion(), vc);
    EXPECT_FALSE(NativeObject::SelfCheck(child, false));
    EXPECT_FALSE(NativeObject::SelfCheck(root, true));

    // 正常写入会刷新哈希，自检恢复通过。
    child->SetInt("a", 2);
    EXPECT_TRUE(NativeObject::SelfCheck(child, false));
    EXPECT_TRUE(NativeObject::SelfCheck(root, true));

    mgr.DestroyGroup(gid);
    FakeluaDeleteState(s);
    NativeObject::SetVersionDebugEnabled(false);
}

TEST(test_native_version, cycle_is_guard) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *a = mgr.Create(gid, "a", 1);
    auto *b = mgr.Create(gid, "b", 2);
    a->SetObject("b", b);
    b->SetObject("a", a);

    const uint64_t vb = b->GetVersion();
    a->SetInt("x", 1);// 不应因环而死循环
    EXPECT_TRUE(b->IsChangedSince(vb));
    EXPECT_TRUE(NativeObject::SelfCheck(a, true));

    mgr.DestroyGroup(gid);
    FakeluaDeleteState(s);
}

TEST(test_native_version, lua_paths_and_container) {
    for (auto jit_type: AllJitTypes()) {
        auto *s = FakeluaNewState();
        auto &mgr = GetNativeObjectManager(s);
        int64_t gid = mgr.CreateGroup();
        static NativeObject *global_player = nullptr;
        global_player = mgr.Create(gid, "player", 100);
        global_player->SetInt("hp", 100);
        RegisterNativeFunction(s, "get_player", 0, false,
                               [](State *state, CVar *, int) -> CVar {
                                   return inter::NativeToFakelua(state, global_player);
                               });

        CompileConfig config;
        CompileFile(s, "./native/test_native_get_set.lua", config);

        // 只读（字段读取 + pairs 快照）不涨版本
        const uint64_t v0 = global_player->GetVersion();
        CVar ret;
        Call(s, jit_type, "test_version_read_only", ret);
        EXPECT_EQ(global_player->GetVersion(), v0);
        inter::Reset(s);

        // 快照已建立后的增量写入（NativeSpecSet snapshot_ready 分支）
        const uint64_t v1 = global_player->GetVersion();
        Call(s, jit_type, "test_version_write_after_pairs", ret);
        EXPECT_TRUE(global_player->IsChangedSince(v1));
        EXPECT_EQ(global_player->GetInt("hp"), 101);
        inter::Reset(s);

        // 容器内部内容写入通过 MarkModified 传播到父对象
        const uint64_t v2 = global_player->GetVersion();
        Call(s, jit_type, "test_version_container_write", ret);
        EXPECT_TRUE(global_player->IsChangedSince(v2));
        auto *container = global_player->GetObject("bag_container");
        ASSERT_NE(container, nullptr);
        const uint64_t vc = container->GetVersion();
        EXPECT_GT(vc, 0);
        inter::Reset(s);

        // 容器只读方法不涨版本
        Call(s, jit_type, "test_version_container_read", ret);
        EXPECT_EQ(inter::FakeluaToNative<int64_t>(s, ret), 1);
        EXPECT_EQ(container->GetVersion(), vc);

        mgr.DestroyGroup(gid);
        global_player = nullptr;
        FakeluaDeleteState(s);
    }
}
