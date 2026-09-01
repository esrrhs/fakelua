package "BasicError"

-- 测试 error 非字符串参数（应报错）
function test_error_non_string()
    local ok, err = pcall(function() error(123) end)
    if ok then return 0 end
    return 1
end

-- 测试 error 表参数（应报错）
function test_error_table()
    local ok, err = pcall(function() error({}) end)
    if ok then return 0 end
    return 1
end
