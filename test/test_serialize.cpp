#include "fakelua.h"
#include "jit/vm.h"
#include "native/native_common.h"
#include "native/serialize/wire_codec.h"
#include "native/table/native_table.h"
#include "test_jit.h"
#include "gtest/gtest.h"

using namespace fakelua;

// 二进制序列化 Lua 绑定测试

TEST(test_serialize, test_int) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_int.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_int", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_float) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_float.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_float", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_bool_nil) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_bool_nil.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_bool_nil", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_string) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_string.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_string", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_table.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_table", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_array_9) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_table.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_array_9", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_cycle_throw) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_table.lua", config);
    int64_t ret = 0;
    CallThrow(s, "SerializeTest.test_cycle_throw", ret);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_decode_too_deep) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_table.lua", config);
    int64_t ret = 0;
    CallThrow(s, "SerializeTest.test_decode_too_deep", ret);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_decode_huge_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_table.lua", config);
    int64_t ret = 0;
    CallThrow(s, "SerializeTest.test_decode_huge_table", ret);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_skip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_skip.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_skip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_text_roundtrip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_boost.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_text_roundtrip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_xml_roundtrip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_boost.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_xml_roundtrip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_text_cycle) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_boost.lua", config);
    int64_t ret = 0;
    CallThrow(s, "SerializeTest.test_text_cycle", ret);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_text_bad) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_boost.lua", config);
    int64_t ret = 0;
    CallThrow(s, "SerializeTest.test_text_bad", ret);
    FakeluaDeleteState(s);
}

TEST(test_serialize, wire_rejects_truncated_overlong_and_unknown_encodings) {
    FakeluaStateGuard state;
    const std::vector<std::string> malformed{{},
                                             std::string("\x03\x80", 2),
                                             std::string("\x03", 1) + std::string(10, '\x80'),
                                             std::string("\x04", 1) + std::string(7, '\0'),
                                             std::string("\x05\x04x", 3),
                                             std::string("\x06\0", 2),
                                             std::string("\xff", 1),
                                             std::string("\0\0", 2)};
    for (const auto &input: malformed) {
        SCOPED_TRACE(::testing::PrintToString(input));
        EXPECT_THROW(serialize::WireDecode(state.GetState(), input), std::exception);
    }
    EXPECT_THROW(serialize::WireDecodeCallbackArgs(state.GetState(), std::string("\0\0", 2)), std::exception);
    EXPECT_TRUE(serialize::WireDecodeCallbackArgs(state.GetState(), {}).vars.empty());
}

TEST(test_serialize, callback_tuple_owns_binary_strings_and_preserves_nil_slots) {
    FakeluaStateGuard state;
    auto *s = state.GetState();
    const std::string binary("a\0b", 3);
    std::vector<CVar> args{inter::NativeToFakeluaNil(s),          inter::NativeToFakeluaBool(s, false),    inter::NativeToFakeluaInt(s, -42),
                           inter::NativeToFakeluaDouble(s, 1.25), inter::NativeToFakeluaString(s, binary), inter::NativeToFakeluaString(s, binary)};
    auto encoded = serialize::WireEncodeCallbackArgs(s, args.data(), 0, args.size(), "test_callback", 1);
    auto decoded = serialize::WireDecodeCallbackArgs(s, encoded);
    encoded.assign(encoded.size(), '?');
    ASSERT_EQ(decoded.vars.size(), args.size());
    EXPECT_EQ(decoded.vars[0].type_, static_cast<int>(VarType::Nil));
    EXPECT_FALSE(inter::CVarToBool(decoded.vars[1], true));
    EXPECT_EQ(inter::CVarToInteger(decoded.vars[2]), -42);
    EXPECT_DOUBLE_EQ(inter::CVarToNumber(decoded.vars[3]), 1.25);
    EXPECT_EQ(inter::CVarToString(decoded.vars[4], ""), binary);
    EXPECT_EQ(inter::CVarToString(decoded.vars[5], ""), binary);
    EXPECT_EQ(serialize::WireDecode(s, serialize::WireEncode(args[0])).type_, static_cast<int>(VarType::Nil));
}

TEST(test_serialize, callback_graph_rejects_cycles_depth_and_nested_closures) {
    FakeluaStateGuard state;
    auto *s = state.GetState();
    CVar cycle = table::TableHelper::CreateTable(s);
    table::TableHelper::SetTableStrId(s, cycle, "self", cycle);
    EXPECT_THROW(serialize::WireEncodeCallbackArgs(s, &cycle, 0, 1, "callback", 1), std::exception);
    EXPECT_THROW(serialize::WireEncode(cycle), std::exception);
    CVar root = table::TableHelper::CreateTable(s);
    CVar tail = root;
    for (int i = 0; i < 65; ++i) {
        CVar next = table::TableHelper::CreateTable(s);
        table::TableHelper::SetTableStrId(s, tail, "next", next);
        tail = next;
    }
    EXPECT_THROW(serialize::WireEncodeCallbackArgs(s, &root, 0, 1, "callback", 1), std::exception);
    CompileConfig config;
    DisableAllJit(config);
    config.disable_jit[JIT_INTERP] = false;
    config.debug_mode = false;
    CompileString(s, "function wire_closure_factory() return function() return 1 end end", config);
    CVar closure = CallByNameArgs(s, JIT_INTERP, "wire_closure_factory", 0, nullptr);
    EXPECT_FALSE(serialize::WireIsSerializable(closure));
    EXPECT_THROW(serialize::WireEncode(closure), std::exception);
    CVar nested = table::TableHelper::CreateTable(s);
    table::TableHelper::SetTableStrId(s, nested, "callback", closure);
    EXPECT_THROW(serialize::WireEncodeCallbackArgs(s, &nested, 0, 1, "callback", 1), std::exception);
}

TEST(test_serialize, unsupported_values_depth_and_malformed_xml_contracts) {
    FakeluaStateGuard state;
    CompileString(state.GetState(), R"(
        function serialization_boundary_contracts()
            local unsupported = function() return 1 end
            for _, encode in ipairs({function(v) return serialize.encode(v) end, function(v) return serialize.text_encode(v) end, function(v) return serialize.xml_encode(v) end}) do
                local ok, message = pcall(encode, unsupported)
                assert(not ok and string.find(message, "unsupported", 1, true))
            end
            local deep = {}
            local cursor = deep
            for i = 1, 65 do local child = {} cursor.next = child cursor = child end
            for _, encode in ipairs({function(v) return serialize.encode(v) end, function(v) return serialize.text_encode(v) end, function(v) return serialize.xml_encode(v) end}) do
                local ok, message = pcall(encode, deep)
                assert(not ok and string.find(message, "nesting too deep", 1, true))
            end
            assert(not pcall(function() return serialize.xml_decode("<not-an-archive>") end))
            local xml = serialize.xml_encode(7)
            local unknown, replacements = string.gsub(xml, "<kind>2</kind>", "<kind>99</kind>")
            assert(replacements == 1)
            local ok, message = pcall(function() return serialize.xml_decode(unknown) end)
            assert(not ok and string.find(message, "unknown kind", 1, true))
            return 1
        end
    )",
                  {});
    int64_t result = 0;
    CallAll(state.GetState(), "serialization_boundary_contracts", result);
    EXPECT_EQ(result, 1);
}

TEST(test_serialize, nullable_host_table_encodes_as_empty_table) {
    FakeluaStateGuard state;
    CVar empty{static_cast<int>(VarType::Table)};
    empty.data_.t = nullptr;
    ASSERT_TRUE(serialize::WireIsSerializable(empty));
    CVar decoded = serialize::WireDecode(state.GetState(), serialize::WireEncode(empty));
    ASSERT_EQ(decoded.type_, static_cast<int>(VarType::Table));
    EXPECT_EQ(table::TableHelper::GetTableLen(decoded), 0);
    size_t entries = 0;
    table::TableHelper::ForEachKV(decoded, [&](CVar, CVar) { ++entries; });
    EXPECT_EQ(entries, 0u);
}
