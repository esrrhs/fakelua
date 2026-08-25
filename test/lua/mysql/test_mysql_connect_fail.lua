package "MysqlTest"

-- 尝试连接一个不存在的端口，验证回调收到错误
function on_connect(conn, err, success)
    conn.err = err
    conn.done = true
end

function test_connect_fail()
    local config = {}
    config["host"] = "127.0.0.1"
    config["port"] = 1
    config["user"] = "root"
    config["password"] = "irrelevant"
    config["db"] = "test"

    local conn = mysql.connect(config, "on_connect")

    -- 驱动 IO 直到回调触发（最多 200 次 tick）
    for i = 1, 200 do
        conn:tick()
        if conn.done then break end
    end

    if not conn.done then
        print("callback never fired")
        return 0
    end

    -- 期望收到错误
    local got_err = conn.err
    if type(got_err) ~= "string" or #got_err == 0 then
        print("expected error message, got:", type(got_err), tostring(got_err))
        return 0
    end

    return 1
end

function on_connect_and_close(conn, err, success)
    conn.closed_ok = true
    conn:close()
end

function test_close_in_connect_cb()
    local config = {}
    config["host"] = "127.0.0.1"
    config["port"] = 1
    config["user"] = "root"
    config["password"] = "irrelevant"
    config["db"] = "test"

    local conn = mysql.connect(config, "on_connect_and_close")
    for i = 1, 200 do
        conn:tick()
        if conn.closed_ok then break end
    end
    if not conn.closed_ok then
        print("close-in-callback never ran")
        return 0
    end
    return 1
end
