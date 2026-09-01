package "BasicXpcall"

-- 测试 xpcall 错误处理函数
function test_xpcall_handler()
    local ok, err = xpcall(function() error("fail") end, function(e)
        return "handled: " .. e
    end)
    if ok then return 0 end
    if not string.find(err, "handled:") then return 0 end
    return 1
end

-- 测试 xpcall 成功
function test_xpcall_success()
    local ok, result = xpcall(function() return 42 end, function(e) return e end)
    if not ok then return 0 end
    return 1
end

-- 测试 xpcall 非函数主调
function test_xpcall_non_function()
    local ok, err = xpcall(function() pcall(function()
        xpcall(123, function() end)
    end) end, function(e) return e end)
    return 1
end
