package "MysqlTest"

-- 验证连接失败时错误信息包含 "connect" 关键字
function test_error_message()
    local ok, err = pcall(function()
        local conn = mysql.connect({
            host = "127.0.0.1",
            port = 1,
            user = "root",
            password = "irrelevant",
            db = "test"
        })
        if conn then conn:close() end
    end)

    if ok then
        print("expected connect to fail, but it succeeded")
        return 0
    end

    -- 错误信息应包含 "connect"
    if not string.find(tostring(err), "connect") then
        print("error message missing 'connect':", tostring(err))
        return 0
    end

    return 1
end
