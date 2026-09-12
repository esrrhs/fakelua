package "RedisTest"

function on_connect(conn, err, success)
    conn.err = err
    conn.ok = success
    conn.done = true
end

function test_connect_fail()
    local config = {}
    config["host"] = "127.0.0.1"
    config["port"] = 1
    config["timeout_ms"] = 500
    local conn = redis.connect(config, "RedisTest.on_connect")
    for i = 1, 1500 do
        runtime.tick()
        if conn.done then break end
        os.sleep(1)
    end
    if not conn.done then return 0 end
    if type(conn.err) ~= "string" or #conn.err == 0 then return 0 end
    if conn.ok ~= 0 then return 0 end
    conn:close()
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
    config["timeout_ms"] = 500
    local conn = redis.connect(config, "RedisTest.on_connect_and_close")
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
    -- TickDepth==0: same teardown as connect_failure_catchable. Needed before
    -- CallAll's next JIT Polls the shared io_context (writer_cv_ expires_at max).
    conn:close()
    return 1
end
