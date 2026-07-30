function test_basic_assert()
    -- assert 成功时返回参数
    local a1 = assert(42)
    if a1 ~= 42 then return 1 end

    -- assert 成功时返回所有参数
    local a2, a3 = assert(true, "msg")
    if a2 ~= true then return 2 end

    -- assert 对非 nil/false 值成功
    local a4 = assert("hello")
    if a4 ~= "hello" then return 3 end

    local a5 = assert({})
    if a5 == nil then return 4 end

    return 5000
end
