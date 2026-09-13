package "RedisTest"

-- 集成测试：连接 + PING + SET/GET
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_cmd(conn, err, result)
    conn.cmd_err = err
    conn.cmd_result = result
    conn.cmd_done = true
end

function test_redis_integration()
    local conn = redis.connect({
        host = "127.0.0.1",
        port = 6379,
        timeout_ms = 1000
    }, "on_connect")

    for i = 1, 1000 do
        runtime.tick()
        if conn.connected or conn.connect_err then break end
        os.sleep(1)
    end

    if not conn.connected then
        local err_str = conn.connect_err or ""
        print("failed to connect:", tostring(err_str))
        return 0
    end

    conn.cmd_done = false
    conn.cmd_err = nil
    conn.cmd_result = nil
    local ping = {}
    ping[1] = "PING"
    conn:command(ping, "on_cmd")

    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end

    if not conn.cmd_done then
        print("PING callback never fired")
        conn:close()
        return 0
    end
    if conn.cmd_err ~= nil then
        if #conn.cmd_err > 0 then
            print("PING failed:", tostring(conn.cmd_err))
            conn:close()
            return 0
        end
    end
    if conn.cmd_result ~= "PONG" then
        print("PING result mismatch:", tostring(conn.cmd_result))
        conn:close()
        return 0
    end

    local key = "fl:redis:it:" .. tostring(os.time())
    conn.cmd_done = false
    conn.cmd_err = nil
    conn.cmd_result = nil
    local setv = {}
    setv[1] = "SET"
    setv[2] = key
    setv[3] = "hello"
    conn:command(setv, "on_cmd")

    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end

    if not conn.cmd_done then
        print("SET callback never fired")
        conn:close()
        return 0
    end
    if conn.cmd_err ~= nil then
        if #conn.cmd_err > 0 then
            print("SET failed:", tostring(conn.cmd_err))
            conn:close()
            return 0
        end
    end
    if conn.cmd_result ~= "OK" then
        print("SET result mismatch:", tostring(conn.cmd_result))
        conn:close()
        return 0
    end

    conn.cmd_done = false
    conn.cmd_err = nil
    conn.cmd_result = nil
    local getv = {}
    getv[1] = "GET"
    getv[2] = key
    conn:command(getv, "on_cmd")

    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end

    if not conn.cmd_done then
        print("GET callback never fired")
        conn:close()
        return 0
    end
    if conn.cmd_err ~= nil then
        if #conn.cmd_err > 0 then
            print("GET failed:", tostring(conn.cmd_err))
            conn:close()
            return 0
        end
    end
    if conn.cmd_result ~= "hello" then
        print("GET result mismatch:", tostring(conn.cmd_result))
        conn:close()
        return 0
    end

    conn.cmd_done = false
    local delv = {}
    delv[1] = "DEL"
    delv[2] = key
    conn:command(delv, "on_cmd")
    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end

    conn:close()
    return 1
end
