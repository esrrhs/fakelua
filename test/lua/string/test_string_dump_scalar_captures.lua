function test_dump_scalar_captures()
    for _, x in ipairs({2.5, true, false, 42}) do
        local original = function() return x end
        local dumped = string.dump(original)
        local restored = load(dumped)
        assert(string.dump(restored) == dumped)
        local callable = load(dumped .. "return x")
        assert(callable() == x)
        local ok, value = pcall(callable)
        assert(ok and value == x)
    end
    local ok, value = xpcall(function() error("expected loaded handler") end, load("return 99"))
    assert(not ok and value == 99)
    assert(load("return missing_loaded_function()")() == nil)
    return 1
end
