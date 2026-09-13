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
    config["timeout_ms"] = 1000

    local conn = mysql.connect(config, "on_connect")

    -- 驱动 IO 直到回调触发（最多 1500 次 tick）
    for i = 1, 1500 do
        runtime.tick()
        if conn.done then break end
        os.sleep(1)
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
    config["timeout_ms"] = 1000

    local conn = mysql.connect(config, "on_connect_and_close")
    for i = 1, 1500 do
        runtime.tick()
        if conn.closed_ok then
            break
        end
        os.sleep(1)
    end
    if not conn.closed_ok then
        return 0
    end
    return 1
end

function test_connect_ssl_require()
    local config = {}
    config["host"] = "127.0.0.1"
    config["port"] = 1
    config["user"] = "root"
    config["password"] = "irrelevant"
    config["db"] = "test"
    config["timeout_ms"] = 1000
    config["ssl"] = true

    local conn = mysql.connect(config, "on_connect")
    for i = 1, 1500 do
        runtime.tick()
        if conn.done then break end
        os.sleep(1)
    end
    if not conn.done then return 0 end
    if type(conn.err) ~= "string" or #conn.err == 0 then return 0 end
    return 1
end
