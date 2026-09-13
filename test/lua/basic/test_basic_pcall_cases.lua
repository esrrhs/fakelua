package "BasicPcallCases"

-- 测试 pcall 非函数参数
function test_pcall_non_function()
    local ok, err = pcall(function() pcall(123) end)
    return 1
end

-- 测试 pcall 函数抛异常
function test_pcall_error()
    local ok, err = pcall(function() error("test error") end)
    if ok then return 0 end
    if not string.find(err, "test error") then return 0 end
    return 1
end

-- 测试 pcall 成功
function test_pcall_success()
    local ok, result = pcall(function() return 42 end)
    if not ok then return 0 end
    return 1
end
