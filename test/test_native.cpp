#include "fakelua.h"
#include "state/state.h"
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
    s->Reset();

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
