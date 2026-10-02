package "MysqlTest"

-- 测试连接生命周期：ping 心跳、重复关闭 (double close) 幂等性、对已关闭连接操作保护
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_result(conn, err, result)
    conn.query_err = err
    conn.query_result = result
    conn.query_done = true
end

function test_lifecycle()
    local conn = mysql.connect({
        host = "127.0.0.1",
        port = 3306,
        user = "root",
        password = "root",
        db = "test"
    }, "on_connect")

    for i = 1, 1000 do
        runtime.tick()
        if conn.connected or conn.connect_err then break end
        os.sleep(1)
    end

    if not conn.connected then
        conn = mysql.connect({
            host = "127.0.0.1",
            port = 3306,
            user = "root",
            password = "",
            db = "test"
        }, "on_connect")

        for i = 1, 1000 do
            runtime.tick()
            if conn.connected or conn.connect_err then break end
            os.sleep(1)
        end
    end

    if not conn.connected then
        local err_str = conn.connect_err or ""
        print("failed to connect:", tostring(err_str))
        return 0
    end

    -- 1. 验证 conn:ping() 心跳
    local sent = conn:ping()
    if not sent then
        print("conn:ping() returned false on open connection")
        conn:close()
        return 0
    end

    -- 驱动 tick 确保 ping 操作完成
    for i = 1, 100 do
        runtime.tick()
        os.sleep(1)
    end

    -- 2. 验证关闭及重复关闭 (double close) 幂等不崩溃
    conn:close()
    conn:close()

    -- 3. 验证对已关闭连接调用 query：不抛错，而是在后续 tick 由回调收到 "closed" 错误
    --    （契约：conn:query 的回调恰好触发一次，连接已关闭时回调收到错误字符串）
    conn.query_done = false
    conn.query_err = nil
    local ok, call_err = pcall(function()
        conn:query("SELECT 1", "on_result")
    end)

    if not ok then
        print("query on closed connection should notify via callback, not throw:", tostring(call_err))
        return 0
    end

    for i = 1, 100 do
        runtime.tick()
        if conn.query_done then break end
        os.sleep(1)
    end

    if not conn.query_done then
        print("closed-connection query callback never fired")
        return 0
    end

    if conn.query_err == nil or not string.find(tostring(conn.query_err), "closed") then
        print("error message should mention 'closed', got:", tostring(conn.query_err))
        return 0
    end

    return 1
end
