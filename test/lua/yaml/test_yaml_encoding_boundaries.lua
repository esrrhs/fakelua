function test_yaml_scalar_encoding()
    assert(yaml.decode(yaml.encode(nil)) == nil)
    assert(yaml.decode(yaml.encode(true)) == true)
    assert(yaml.decode(yaml.encode(false)) == false)
    assert(yaml.decode(yaml.encode(1.25)) == 1.25)
    assert(yaml.decode(yaml.encode(1 / 0)) == nil)
    assert(yaml.decode(yaml.encode(0 / 0)) == nil)
    assert(yaml.decode("'a''b[[{' ") == "a'b[[{")
    assert(yaml.decode('"a\\\"[[{"') == 'a"[[{')
    assert(yaml.decode("value: 7 # [[[{{{\n").value == 7)
    assert(yaml.decode("999999999999999999999999999999999999") == "999999999999999999999999999999999999")
    return 1
end

function test_yaml_encoding_rejections()
    local cycle = {}
    cycle[1] = cycle
    local ok, err = pcall(function() return yaml.encode(cycle) end)
    assert(not ok and string.find(err, "cyclic", 1, true) ~= nil)
    local nested = {1}
    for i = 1, 66 do nested = {nested} end
    ok, err = pcall(function() return yaml.encode(nested) end)
    assert(not ok and string.find(err, "nesting too deep", 1, true) ~= nil)
    ok, err = pcall(function() return yaml.encode(function() return 1 end) end)
    assert(not ok and string.find(err, "unsupported type", 1, true) ~= nil)
    local block = ""
    for i = 0, 66 do block = block .. string.rep("  ", i) .. "key:\n" end
    ok, err = pcall(function() return yaml.decode(block) end)
    assert(not ok and string.find(err, "nesting too deep", 1, true) ~= nil)
    return 1
end
