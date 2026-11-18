function test_random_contracts()
    local a, b = random.new(123), random.new(123)
    for i = 1, 20 do
        assert(a:int(-9223372036854775807 - 1, 9223372036854775807) == b:int(-9223372036854775807 - 1, 9223372036854775807))
    end
    a:set_state("0xabcdef0123456789")
    assert(a:get_state() == "0xABCDEF0123456789")
    assert(not pcall(function() a:set_state(false) end))
    assert(not pcall(function() a:weighted(false) end))
    assert(not pcall(function() a:dice(2, 9223372036854775807) end))
    return 1
end
