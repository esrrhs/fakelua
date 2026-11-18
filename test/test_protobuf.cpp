#include "fakelua.h"
#include "native/protobuf/protobuf_parser.h"
#include "native/protobuf/protobuf_schema.h"
#include "test_jit.h"
#include "gtest/gtest.h"
#include <limits>

using namespace fakelua;

// Protobuf 绑定测试

TEST(test_protobuf, test_load) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_load.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProtobufTest.test_load", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_scalar) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_scalar.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProtobufTest.test_scalar", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_wrong_wire_type) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_scalar.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProtobufTest.test_wrong_wire_type", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_message) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_message.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProtobufTest.test_message", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_repeated) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_repeated.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProtobufTest.test_repeated", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_map) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_map.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProtobufTest.test_map", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_enum) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_enum.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProtobufTest.test_enum", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_map_enum) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_enum.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProtobufTest.test_map_enum", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_cycle_throw) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_message.lua", config);
    int64_t ret = 0;
    CallThrow(s, "ProtobufTest.test_cycle_throw", ret);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_decode_too_deep) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_message.lua", config);
    int64_t ret = 0;
    CallThrow(s, "ProtobufTest.test_decode_too_deep", ret);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_optional_nil) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_message.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProtobufTest.test_optional_nil", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, parser_nested_options_and_type_resolution) {
    FakeluaStateGuard state;
    auto *s = state.GetState();
    const std::string schema = R"proto(
        // Imports are declarations; callers load dependencies explicitly.
        syntax = 'proto3'; package coverage.proto;
        import public "public.proto"; import weak 'weak.proto'; ;
        option java_package = "a\n\t\r\\\'\"\0\x";
        option (custom.(nested)) = 1.25;
        /* A multi-line comment
           must not alter declaration parsing. */
        enum Status { ; option allow_alias = 1; OK = 0 [deprecated = true]; };
        message Outer {
            ; option deprecated = 1;
            reserved 20 to 29, "old"; extensions 100 to 199;
            message Inner { string text = 1; };
            enum Kind { ; ZERO = 0; ONE = 0x1; }
            optional Inner child = 1 [deprecated = true];
            repeated coverage.proto.Outer.Inner children = 2;
            Status status = 3;
            Kind kind = 4;
            map<string, Inner> by_name = 5 [deprecated = true];
            map<int32, bool> flags = 6;
            oneof choice {
                ; option (custom.choice) = "selected";
                string title = 7 [deprecated = true];
                Inner selected = 8;
            }
        };
    )proto";
    ASSERT_TRUE(protobuf::ParseProto(s, schema, "nested.proto").empty());
    auto &registry = protobuf::GetProtobufState(s);
    const auto *outer = registry.FindMessage("coverage.proto.Outer");
    ASSERT_NE(outer, nullptr);
    ASSERT_EQ(outer->fields.size(), 8u);
    EXPECT_TRUE(outer->fields[0].optional);
    EXPECT_EQ(outer->fields[0].type_name, "coverage.proto.Outer.Inner");
    EXPECT_TRUE(outer->fields[1].repeated);
    EXPECT_EQ(outer->fields[1].type_name, "coverage.proto.Outer.Inner");
    EXPECT_EQ(outer->fields[2].type, protobuf::TYPE_ENUM);
    EXPECT_EQ(outer->fields[3].type, protobuf::TYPE_ENUM);
    EXPECT_TRUE(outer->fields[4].is_map);
    EXPECT_EQ(outer->fields[4].map_value_type_name, "coverage.proto.Outer.Inner");
    EXPECT_EQ(outer->fields[5].map_value_type, protobuf::TYPE_BOOL);
    EXPECT_GE(outer->fields[6].oneof_index, 0);
    EXPECT_EQ(outer->fields[6].oneof_index, outer->fields[7].oneof_index);
    ASSERT_NE(registry.FindEnum("coverage.proto.Outer.Kind"), nullptr);
    EXPECT_EQ(registry.FindEnum("coverage.proto.Outer.Kind")->number_to_name.at(1), "ONE");
}

TEST(test_protobuf, parser_reports_malformed_declarations) {
    struct InvalidSchema {
        const char *text;
        const char *message;
    };

    const InvalidSchema cases[] = {
            {"@", "unexpected character"},
            {"syntax = 3;", "expected syntax string"},
            {"syntax 'proto3';", "expected '='"},
            {"optional", "unexpected token"},
            {"message {}", "expected message name"},
            {"message M { 1 x = 1; }", "expected field type"},
            {"message M { string = 1; }", "expected field name"},
            {"message M { string x = name; }", "expected field number"},
            {"message M { string x = 99999999999999999999999999; }", "invalid field number"},
            {"message M { foo. = 1; }", "expected identifier after '.'"},
            {"message M { map<1, string> x = 1; }", "expected map key type"},
            {"message M { map<string, 1> x = 1; }", "expected type name"},
            {"message M { map<string, int32> = 1; }", "expected map field name"},
            {"message M { map<not_scalar, int32> x = 1; }", "unknown proto scalar type"},
            {"message M { oneof { } }", "expected oneof name"},
            {"message M { oneof choice { int32 = 1; } }", "expected field name"},
            {"enum {}", "expected enum name"},
            {"enum E { 1 = 2; }", "expected enum value name"},
            {"message M { string x = 1 [", "expected ';'"},
            {"message M { map<string, int32> x = 1 [", "expected ';'"},
            {"message M { oneof choice { string x = 1 [", "expected ';'"},
            {"enum E { X = 1 [", "expected ';'"},
            {"message M { reserved 2", "expected ';'"},
            {"message M { extensions 2", "expected ';'"},
            {"option (unfinished", "expected ';'"},
    };
    for (const auto &test: cases) {
        SCOPED_TRACE(test.text);
        FakeluaStateGuard state;
        const auto error = protobuf::ParseProto(state.GetState(), test.text, "invalid.proto");
        EXPECT_NE(error.find(test.message), std::string::npos) << error;
    }
}

TEST(test_protobuf, schema_registry_replacement_and_state_isolation) {
    FakeluaStateGuard first, second;
    auto &a = protobuf::GetProtobufState(first.GetState());
    auto &b = protobuf::GetProtobufState(second.GetState());
    ASSERT_TRUE(protobuf::ParseProto(first.GetState(), "message M { string later = 3; int32 earlier = 1; }", "first.proto").empty());
    ASSERT_NE(a.FindMessage("M"), nullptr);
    EXPECT_EQ(a.FindMessage("M")->number_to_field.at(1)->name, "earlier");
    EXPECT_EQ(a.FindMessage("M")->number_to_field.at(3)->name, "later");
    EXPECT_EQ(b.FindMessage("M"), nullptr);
    EXPECT_EQ(b.FindEnum("E"), nullptr);
    ASSERT_TRUE(protobuf::ParseProto(first.GetState(), "enum E { ZERO = 0; } message M { E status = 2; }", "replacement.proto").empty());
    ASSERT_NE(a.FindMessage("M"), nullptr);
    ASSERT_EQ(a.FindMessage("M")->fields.size(), 1u);
    EXPECT_EQ(a.FindMessage("M")->fields[0].type, protobuf::TYPE_ENUM);
    EXPECT_EQ(a.MessageNames(), std::vector<std::string>{"M"});
    a.Clear();
    EXPECT_TRUE(a.MessageNames().empty());
    EXPECT_EQ(a.FindEnum("E"), nullptr);
}

TEST(test_protobuf, wire_scalar_boundaries_and_truncated_inputs) {
    using namespace fakelua::protobuf;
    for (uint64_t value: {uint64_t{0}, uint64_t{127}, uint64_t{128}, uint64_t{16384}, std::numeric_limits<uint64_t>::max()}) {
        std::string encoded;
        WriteVarint(encoded, value);
        size_t position = 0;
        EXPECT_EQ(ReadVarint(encoded, position), value);
        EXPECT_EQ(position, encoded.size());
        position = 0;
        SkipValue(encoded, position, WIRE_VARINT);
        EXPECT_EQ(position, encoded.size());
    }
    for (const auto &truncated: {std::string{}, std::string(1, '\x80'), std::string(10, '\x80')}) {
        size_t position = 0;
        EXPECT_THROW(ReadVarint(truncated, position), std::exception);
    }
    for (size_t length = 0; length < 8; ++length) {
        const std::string data(length, '\0');
        size_t position = 0;
        EXPECT_THROW(ReadFixed64(data, position), std::exception);
        position = 0;
        EXPECT_THROW(SkipValue(data, position, WIRE_64BIT), std::exception);
        if (length < 4) {
            position = 0;
            EXPECT_THROW(ReadFixed32(data, position), std::exception);
            position = 0;
            EXPECT_THROW(SkipValue(data, position, WIRE_32BIT), std::exception);
        }
    }
    for (uint8_t wire: {uint8_t{3}, uint8_t{4}, uint8_t{6}, uint8_t{7}}) {
        size_t position = 0;
        EXPECT_THROW(SkipValue("", position, wire), std::exception);
    }
    size_t position = std::numeric_limits<size_t>::max();
    EXPECT_THROW(ReadFixed32("data", position), std::exception);
    position = std::numeric_limits<size_t>::max();
    EXPECT_THROW(ReadFixed64("data", position), std::exception);
    for (auto wire: {WIRE_32BIT, WIRE_64BIT}) {
        std::string encoded;
        if (wire == WIRE_32BIT) WriteFixed32(encoded, 0x12345678);
        else
            WriteFixed64(encoded, 0x123456789abcdef0ull);
        position = 0;
        SkipValue(encoded, position, wire);
        EXPECT_EQ(position, encoded.size());
    }
    const std::string length_delimited("\x03"
                                       "abc",
                                       4);
    position = 0;
    EXPECT_EQ(ReadLengthDelimited(length_delimited, position), "abc");
    EXPECT_EQ(position, length_delimited.size());
    position = 0;
    SkipValue(length_delimited, position, WIRE_LEN);
    EXPECT_EQ(position, length_delimited.size());
    position = 0;
    EXPECT_THROW(ReadLengthDelimited(std::string("\x04"
                                                 "abc",
                                                 4),
                                     position),
                 std::exception);
    position = 0;
    EXPECT_THROW(SkipValue(std::string("\x04"
                                       "abc",
                                       4),
                           position, WIRE_LEN),
                 std::exception);
}

TEST(test_protobuf, nested_message_containers_and_complete_field_metadata) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./protobuf/test_protobuf_nested_containers.lua", {});
    for (auto backend: AllJitTypes()) {
        int64_t result = 0;
        Call(state.GetState(), backend, "test_protobuf_nested_containers", result);
        EXPECT_EQ(result, 1) << JitTypeName(backend);
        Call(state.GetState(), backend, "test_protobuf_metadata_contracts", result);
        EXPECT_EQ(result, 1) << JitTypeName(backend);
    }
}

TEST(test_protobuf, scalar_repeated_compatibility_unknown_map_fields_and_nesting_limit) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./protobuf/test_protobuf_wire_compatibility.lua", {});
    int64_t result = 0;
    CallAll(state.GetState(), "test_protobuf_wire_compatibility", result);
    EXPECT_EQ(result, 1);
}
