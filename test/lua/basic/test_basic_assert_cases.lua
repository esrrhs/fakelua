package "BasicAssertCases"

-- 测试 assert 非字符串 message（应报错）
function test_assert_bad_message()
    local ok, err = pcall(function() assert(false, 123) end)
    if ok then return 0 end
    return 1
end

-- 测试 assert 表作为 message（应报错）
function test_assert_table_message()
    local ok, err = pcall(function() assert(false, {}) end)
    if ok then return 0 end
    return 1
end

-- 测试 assert 成功时返回所有参数
function test_assert_success_multi()
    local a, b, c = assert(true, "msg", 42, "extra")
    if a ~= true then return 0 end
    if b ~= "msg" then return 0 end
    if c ~= 42 then return 0 end
    return 1
end
