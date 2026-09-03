package "MysqlTest"

-- 连接池测试（需要本地 MySQL 服务）
function on_pool_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_pool_result(conn, err, result)
    conn.query_err = err
    if result and result[3] and result[3][1] then
        conn.result_first_value = result[3][1][1]
    else
        conn.result_first_value = nil
    end
    conn.query_done = true
end

function test_pool()
    local pool = mysql_pool.create({
        host = "127.0.0.1",
        port = 3306,
        user = "root",
        password = "root",
        db = "test",
        pool_size = 2,
        heartbeat_ms = 0  -- disable heartbeat for test
    })

    -- 驱动连接池直到连接建立（最多 200 次 tick）
    local conn = nil
    for i = 1, 1000 do
        pool:tick()
        conn = pool:acquire()
        if conn then break end
    end

    if not conn then
        print("pool: no connection available (no MySQL server?)")
        return 1  -- skip without MySQL
    end

    -- 验证连接可用
    conn.query_done = false
    conn.query_err = nil
    conn.result_first_value = nil
    conn:query("SELECT 1 AS test", "on_pool_result")

    for i = 1, 1000 do
        pool:tick()
        if conn.query_done then break end
    end

    if type(conn.query_err) == "string" then
        if #conn.query_err > 0 then
            print("pool query failed:", conn.query_err)
            pool:release(conn)
            pool:close()
            return 0
        end
    end

    -- 验证结果
    if conn.result_first_value ~= "1" then
        print("pool query result mismatch:", conn.result_first_value)
        pool:release(conn)
        pool:close()
        return 0
    end

    -- 释放连接
    pool:release(conn)

    -- 验证统计
    local stats = pool:stats()
    if stats[1] ~= 2 then
        print("pool total expected 2, got:", stats[1])
        pool:close()
        return 0
    end

    pool:close()
    return 1
end
