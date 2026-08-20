package "MysqlTest"

-- 验证连接失败时错误信息包含 "connect" 关键字
function on_connect_msg(conn, err, success)
    conn.err = err
    conn.done = true
end

function test_error_message()
    local config = {}
    config["host"] = "127.0.0.1"
    config["port"] = 1
    config["user"] = "root"
    config["password"] = "irrelevant"
    config["db"] = "test"

    local conn = mysql.connect(config, "on_connect_msg")

    for i = 1, 200 do
        conn:tick()
        if conn.done then break end
    end

    if not conn.done then
        print("callback never fired")
        return 0
    end

    -- 错误信息应包含 "connect"
    local got_err = conn.err
    if not string.find(tostring(got_err), "connect") then
        print("error message missing 'connect':", tostring(got_err))
        return 0
    end

    return 1
end
