function test_protobuf_wire_compatibility()
    assert(protobuf.load([[
        message WireChild { int32 value = 1; WireChild child = 2; }
        message WireCompatible {
            repeated WireChild items = 1;
            repeated string names = 2;
            repeated double numbers = 3;
            map<string,int32> entries = 4;
        }
    ]]) == "ok")
    local decoded = protobuf.decode("WireCompatible", protobuf.encode("WireCompatible", {items = {value = 23}, names = "one", numbers = 7}))
    assert(decoded.items[1].value == 23 and decoded.names[1] == "one" and decoded.numbers[1] == 7)
    local valid = protobuf.decode("WireCompatible", string.char(34, 7, 10, 1, 97, 16, 7, 24, 1, 72, 1))
    assert(valid.entries.a == 7)
    local bad_key = protobuf.decode("WireCompatible", string.char(34, 4, 8, 1, 16, 7))
    assert(next(bad_key.entries) == nil)
    local bad_value = protobuf.decode("WireCompatible", string.char(34, 6, 10, 1, 97, 18, 1, 98))
    assert(next(bad_value.entries) == nil)
    local mixed = protobuf.decode("WireCompatible", protobuf.encode("WireCompatible", {items = {99, value = 31}, names = {}, numbers = {}}))
    assert(mixed.items[1].value == 31 and mixed.names == nil and mixed.numbers == nil)
    local root, cursor = {}, nil
    cursor = root
    for i = 1, 70 do cursor.child = {} cursor = cursor.child end
    local ok, message = pcall(function() protobuf.encode("WireChild", root) end)
    assert(not ok and string.find(message, "nesting too deep", 1, true))
    return 1
end
