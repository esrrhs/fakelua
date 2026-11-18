function test_ini_encoding_contracts()
    local value = {z = {flag = false}, ["a[\n"] = {["key=\0"] = 42, float = 1.25, flag = true, list = {[2] = "b", [1] = "a", z = true, y = 1.25}}}
    local encoded = ini.encode(value)
    assert(string.find(encoded, "[a__]", 1, true) ~= nil)
    assert(string.find(encoded, "key__ = 42", 1, true) ~= nil)
    assert(string.find(encoded, "list = a, b, 1.25, true", 1, true) ~= nil)
    local decoded = ini.decode(encoded)
    assert(decoded["a__"].float == 1.25 and decoded["a__"].flag == true and decoded.z.flag == false)
    local scalars = ini.decode("[x]\nvalue=null\nleading=001\n")
    assert(scalars.x.value == nil and scalars.x.leading == "001")
    assert(not pcall(function() return ini.decode("a=one\0b=two") end))
    assert(not pcall(function() return ini.decode("[unfinished\n") end))
    assert(not pcall(function() return ini.encode(1) end))
    return 1
end
