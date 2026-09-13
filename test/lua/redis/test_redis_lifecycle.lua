package "RedisTest"

-- 测试连接生命周期：重复关闭幂等、对已关闭连接 command 防护、空 argv
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_cmd(conn, err, result)
    conn.cmd_err = err
    conn.cmd_result = result
    conn.cmd_done = true
end

function test_lifecycle()
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

    -- 1. 空 argv 应受控抛错
    local ok_empty, err_empty = pcall(function()
        local empty = {}
        conn:command(empty, "on_cmd")
    end)
    if ok_empty then
        print("expected error for empty argv, but pcall succeeded")
        conn:close()
        return 0
    end
    if not string.find(tostring(err_empty), "empty") then
        print("empty argv error should mention 'empty', got:", tostring(err_empty))
        conn:close()
        return 0
    end

    -- 2. 关闭及重复关闭幂等不崩溃
    conn:close()
    conn:close()

    -- 3. 对已关闭连接调用 command 能够受控捕获错误
    local ok, err_msg = pcall(function()
        local argv = {}
        argv[1] = "PING"
        conn:command(argv, "on_cmd")
    end)

    if ok then
        print("expected error when commanding closed connection, but pcall succeeded")
        return 0
    end

    if not string.find(tostring(err_msg), "closed") then
        print("error message should mention 'closed', got:", tostring(err_msg))
        return 0
    end

    return 1
end
