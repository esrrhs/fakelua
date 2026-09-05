package "MysqlTest"

-- 测试连接池边界情况：连接耗尽 (exhaustion) 与通过 conn:close() 归还
function test_pool_advanced()
    local pool = mysql_pool.create({
        host = "127.0.0.1",
        port = 3306,
        user = "root",
        password = "root",
        db = "test",
        pool_size = 2,
        heartbeat_ms = 0
    })

    -- 驱动连接池直到连接建立
    local conn1 = nil
    for i = 1, 1000 do
        pool:tick()
        conn1 = pool:acquire()
        if conn1 then break end
    end

    if not conn1 then
        print("pool_adv: no connection available (skip without server)")
        return 1
    end

    -- 借出第 2 个连接
    local conn2 = nil
    for i = 1, 1000 do
        pool:tick()
        conn2 = pool:acquire()
        if conn2 then break end
    end

    if not conn2 then
        print("pool_adv: failed to acquire second connection")
        pool:release(conn1)
        pool:close()
        return 0
    end

    -- 1. 验证连接池耗尽：pool_size=2 均已借出，第 3 次 acquire 应返回 nil
    local conn3 = pool:acquire()
    if conn3 ~= nil then
        print("expected nil when pool exhausted, got connection")
        pool:release(conn1)
        pool:release(conn2)
        pool:release(conn3)
        pool:close()
        return 0
    end

    -- 2. 验证通过 conn1:close() 归还连接（pool wrap 对象上 close 映射为 release）
    conn1:close()

    -- 3. 归还后应能重新借出连接
    local conn_reacquired = pool:acquire()
    if not conn_reacquired then
        print("failed to acquire connection after conn1:close()")
        pool:release(conn2)
        pool:close()
        return 0
    end

    -- 归还并关闭
    conn_reacquired:close()
    conn2:close()
    pool:close()
    return 1
end
