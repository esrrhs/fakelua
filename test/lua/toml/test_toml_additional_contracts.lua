function test_toml_contracts()
    assert(toml.encode(nil) == "")
    assert(toml.encode(function() return 1 end) == "")
    local decoded = toml.decode(toml.encode({[true] = "skip", child = {[false] = "skip", value = 7}, callable = function() end}))
    assert(decoded.child.value == 7 and decoded.callable == nil)
    local root = {}
    local cursor = root
    for i = 1, 70 do cursor.child = {} cursor = cursor.child end
    local ok, message = pcall(function() toml.encode(root) end)
    assert(not ok and string.find(message, "nesting too deep", 1, true))
    return 1
end
