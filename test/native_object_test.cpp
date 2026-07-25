#include <gtest/gtest.h>
#include "fakelua.h"
#include "state/state.h"
#include <unordered_map>

using namespace fakelua;

TEST(NativeObjectTest, BasicKVAndTypes) {
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

TEST(NativeObjectTest, NestedObject) {
    auto* player = NativeObject::Create("player");
    auto* bag = NativeObject::Create("bag");

    bag->SetInt("gold", 999);
    player->SetObject("bag", bag);

    EXPECT_EQ(player->GetObject("bag"), bag);
    EXPECT_EQ(player->GetObject("bag")->GetInt("gold"), 999);

    NativeObject::Destroy(bag);
    NativeObject::Destroy(player);
}

TEST(NativeObjectTest, FullyDynamicPropertyAndBuiltinApi) {
    auto* s = FakeluaNewState();

    CompileConfig config;
    CompileString(s, R"(
        local msg_handlers = {}

        -- 登录：使用内置 new_native_obj(type, id) 创建 C++ 持久对象
        -- 完全不需要在 C++ 提前定义任何字段，直接点号读写！
        msg_handlers["on_login"] = function(pid, pname)
            local player = new_native_obj("player", pid)
            player.hp = 100
            player.mp = 200
            player.name = pname
            return player.hp
        end

        -- 业务请求：使用内置 get_native_obj(type, id) 获取对象
        -- 纯点号赋值/读取：player.hp / player.last_talk
        msg_handlers["on_talk"] = function(pid, words)
            local player = get_native_obj("player", pid)
            if player == nil then
                return "not_found"
            end

            -- 扣减 20 点 hp，新增一个动态字段 last_talk
            player.hp = player.hp - 20
            player.last_talk = words

            return player.name .. " (" .. player.hp .. "hp): " .. player.last_talk
        end

        -- 统一消息入口
        function on_msg(msg_name, ...)
            local handler = msg_handlers[msg_name]
            if handler == nil then
                return "unknown_msg"
            end
            return handler(...)
        end
    )", config);

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
