function test_pool_lifecycle_contracts()
    assert(not pcall(function() mysql_pool.create(false) end))
    for _, ssl in ipairs({false, true, 0, 1, "require", "true", "enable", "disable"}) do
        local pool = mysql_pool.create({host = "127.0.0.1", port = 1,
            user = "root", password = "invalid", db = "test", pool_size = 1,
            ssl = ssl, ssl_ca = "", max_retries = 0, heartbeat_ms = 0})
        assert(pool:stats()[1] == 1)
        assert(pool:acquire() == nil)
        assert(pool:release(false) == nil)
        assert(pool:release({}) == nil)
        pool:close()
        pool:close()
    end
    local pool = mysql_pool.create({host = "127.0.0.1", port = 3306,
        user = "root", password = "root", db = "test", pool_size = 1,
        ssl = false, heartbeat_ms = 0})
    local conn = nil
    for i = 1, 5000 do
        runtime.tick()
        conn = pool:acquire()
        if conn then break end
        os.sleep(1)
    end
    assert(conn ~= nil)
    local err = conn:error()
    assert(type(err) == "table" and err[1] == 0 and err[2] == 0)
    -- Closing the owner while a wrapper is borrowed invalidates the wrapper,
    -- not only the pool's internal connection storage.
    pool:close()
    assert(conn:error() == nil)
    local ok, message = pcall(function() conn:query("SELECT 1", "unused_pool_callback") end)
    assert(not ok and string.find(message, "closed", 1, true))
    conn:close()
    pool:release(conn)
    pool:close()
    return 1
end
