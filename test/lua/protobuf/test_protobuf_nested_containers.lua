function test_protobuf_nested_containers()
    assert(protobuf.load([[
        syntax = "proto3";
        message ContainerItem { int32 value = 1; }
        message Container {
            map<string, ContainerItem> entries = 1;
            repeated ContainerItem items = 2;
            ContainerItem child = 3;
            repeated int32 numbers = 4 [packed = false];
        }
    ]]) == "ok")
    local value = {
        entries = { first = {value = 7}, second = {value = 9} },
        items = {{value = 11}, {value = 13}},
        child = {value = 15}, numbers = {1, -2, 3}
    }
    local decoded = protobuf.decode("Container", protobuf.encode("Container", value))
    assert(decoded.entries.first.value == 7 and decoded.entries.second.value == 9)
    assert(#decoded.items == 2 and decoded.items[1].value == 11 and decoded.items[2].value == 13)
    assert(decoded.child.value == 15 and decoded.numbers[2] == -2)
    local empty = protobuf.decode("Container", protobuf.encode("Container", {items = {}, numbers = {}}))
    assert(empty.child == nil)
    return 1
end

function test_protobuf_metadata_contracts()
    assert(protobuf.load([[
        enum MetadataEnum { ZERO = 0; }
        message MetadataChild { int32 value = 1; }
        message MetadataKinds {
            double a = 1; float b = 2; int64 c = 3; uint64 d = 4;
            int32 e = 5; fixed64 f = 6; fixed32 g = 7; bool h = 8;
            string i = 9; bytes j = 10; uint32 k = 11; MetadataEnum l = 12;
            sfixed32 m = 13; sfixed64 n = 14; sint32 o = 15; sint64 p = 16;
            MetadataChild q = 17;
        }
    ]]) == "ok")
    local expected = {"double", "float", "int64", "uint64", "int32", "fixed64", "fixed32", "bool", "string", "bytes", "uint32", "enum", "sfixed32", "sfixed64", "sint32", "sint64", "message"}
    local fields = protobuf.fields("MetadataKinds")
    assert(#fields == #expected)
    for i = 1, #expected do
        assert(fields[i].number == i and fields[i].type == expected[i])
    end
    assert(fields[12].type_name == "MetadataEnum" and fields[17].type_name == "MetadataChild")
    assert(not pcall(function() return protobuf.load() end))
    assert(not pcall(function() return protobuf.encode() end))
    assert(not pcall(function() return protobuf.decode() end))
    assert(not pcall(function() return protobuf.fields() end))
    assert(not pcall(function() return protobuf.fields("NotDefined") end))
    assert(not pcall(function() return protobuf.encode("NotDefined", {}) end))
    assert(not pcall(function() return protobuf.decode("NotDefined", "") end))
    return 1
end
