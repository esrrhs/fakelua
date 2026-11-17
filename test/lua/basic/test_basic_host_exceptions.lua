function test_host_exception_boundaries()
    local ok, message = pcall(function() host_standard_error() end)
    assert(not ok and string.find(message, "host failure", 1, true))
    ok, message = pcall(function() host_unknown_error() end)
    assert(not ok and string.find(message, "unknown", 1, true))
    ok, message = xpcall(function() error("original failure") end, function(err) host_standard_error() end)
    assert(not ok and string.find(message, "original failure", 1, true))
    assert(not pcall(function() xpcall(function() return 1 end, 123) end))
    local key = {}
    local iterator, state, control = pairs({[key] = 9})
    local returned, value = iterator(state, control)
    assert(returned == key and value == 9)
    assert(iterator(state, returned) == nil)
    -- Protected-call failures must not poison the next ordinary call.
    local success, result = pcall(function() return 42 end)
    assert(success and result == 42)
    return 1
end
