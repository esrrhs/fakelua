package "RedisTest"

-- 测试命令错误及连接恢复能力
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_cmd(conn, err, result)
    conn.cmd_err = err
    conn.cmd_result = result
    conn.cmd_done = true
end

function test_cmd_error()
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

    -- 1. 参数个数错误的 GET
    conn.cmd_done = false
    conn.cmd_err = nil
    conn.cmd_result = nil
    local bad = {}
    bad[1] = "GET"
    conn:command(bad, "on_cmd")

    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end

    if not conn.cmd_done then
        print("cmd_error: callback never fired")
        conn:close()
        return 0
    end

    local failed = false
    if conn.cmd_err ~= nil then
        if type(conn.cmd_err) == "string" and #conn.cmd_err > 0 then
            failed = true
        end
    end
    if not failed then
        if type(conn.cmd_result) == "string" then
            local msg = tostring(conn.cmd_result)
            if string.find(msg, "ERR") or string.find(msg, "wrong") or string.find(msg, "arg") then
                failed = true
            end
        end
    end
    if not failed then
        print("expected error for GET without key, err=", tostring(conn.cmd_err), "result=", tostring(conn.cmd_result))
        conn:close()
        return 0
    end

    -- 2. 出错后连接恢复，后续 SET/GET 仍可用
    local key = "fl:redis:err:" .. tostring(os.time())
    conn.cmd_done = false
    conn.cmd_err = nil
    conn.cmd_result = nil
    local setv = {}
    setv[1] = "SET"
    setv[2] = key
    setv[3] = "recovered"
    conn:command(setv, "on_cmd")

    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end

    if not conn.cmd_done then
        print("recovery SET: callback never fired")
        conn:close()
        return 0
    end
    if conn.cmd_err ~= nil then
        if #conn.cmd_err > 0 then
            print("recovery SET failed:", tostring(conn.cmd_err))
            conn:close()
            return 0
        end
    end
    if conn.cmd_result ~= "OK" then
        print("recovery SET result mismatch:", tostring(conn.cmd_result))
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
        print("recovery GET: callback never fired")
        conn:close()
        return 0
    end
    if conn.cmd_err ~= nil then
        if #conn.cmd_err > 0 then
            print("recovery GET failed:", tostring(conn.cmd_err))
            conn:close()
            return 0
        end
    end
    if conn.cmd_result ~= "recovered" then
        print("recovery GET result mismatch:", tostring(conn.cmd_result))
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
