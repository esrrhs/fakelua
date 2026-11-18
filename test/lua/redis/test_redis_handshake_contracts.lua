package "RedisHandshake"

function on_connect(conn, err, success)
    conn.connect_done = true
    conn.connected = (success == 1)
    conn.connect_error = err
end

function on_command(conn, err, result)
    conn.command_done = true
    conn.command_error = err
    conn.command_result = result
    if conn.close_after_result then conn:close() end
end

local function await_connection(conn)
    for i = 1, 3000 do
        runtime.tick()
        if conn.connect_done then break end
        os.sleep(1)
    end
    assert(conn.connect_done)
end

local function command(conn, args)
    conn.command_done = false
    conn.command_error = nil
    conn:command(args, "RedisHandshake.on_command")
    for i = 1, 3000 do
        runtime.tick()
        if conn.command_done then break end
        os.sleep(1)
    end
    assert(conn.command_done)
end

function test_handshake_contracts()
    local users = {"default", "fakelua_nonexistent_user"}
    for i = 1, #users do
        local failed = redis.connect({host = "127.0.0.1", port = 6379, user = users[i], password = "fakelua_intentionally_invalid", timeout_ms = 1000}, "RedisHandshake.on_connect")
        await_connection(failed)
        local accepted = failed.connected
        local message = failed.connect_error
        failed:close()
        assert(not accepted and type(message) == "string" and #message > 0)
    end
    local conn = redis.connect({host = "127.0.0.1", port = 6379, db = 1, timeout_ms = 1000}, "RedisHandshake.on_connect")
    await_connection(conn)
    assert(conn.connected, conn.connect_error)
    local ok, err = pcall(function()
        command(conn, {"PING"})
        assert(conn.command_error == nil and conn.command_result == "PONG")
        command(conn, {"HELLO", "3"})
        if conn.command_error == nil then
            assert(type(conn.command_result) == "table" and conn.command_result.proto == 3)
            command(conn, {"EVAL", "redis.setresp(3); return true", "0"})
            assert(conn.command_error == nil and conn.command_result == true)
            command(conn, {"EVAL", "redis.setresp(3); return {double = 1.25}", "0"})
            assert(conn.command_error == nil and conn.command_result == 1.25)
        else
            -- The Windows CI Redis 5 service predates HELLO/RESP3.
            assert(string.find(conn.command_error, "unknown", 1, true) ~= nil)
        end
        assert(not pcall(function() return conn:command(1, "RedisHandshake.on_command") end))
        conn.close_after_result = true
        command(conn, {"PING"})
        assert(conn.command_error == nil and conn.command_result == "PONG")
    end)
    conn:close()
    assert(ok, err)
    assert(not pcall(function() return redis.connect(1, "RedisHandshake.on_connect") end))
    return 1
end


function test_pending_connection_and_deep_reply_contracts()
    local conn = redis.connect({host = "127.0.0.1", port = 6379}, "RedisHandshake.on_connect")
    assert(not pcall(function() conn:command({"PING"}, "RedisHandshake.on_command") end))
    await_connection(conn)
    assert(conn.connected, conn.connect_error)
    local ok, message = pcall(function()
        command(conn, {"EVAL", "local t = {}; local p = t; for i = 1, 70 do local n = {}; p[1] = n; p = n end; return t", "0"})
        assert(type(conn.command_error) == "string")
        assert(string.find(conn.command_error, "deep", 1, true))
        command(conn, {"PING"})
        assert(conn.command_error == nil and conn.command_result == "PONG")
        for i = 1, 16 do conn:command({"PING"}, "RedisHandshake.on_command") end
    end)
    conn:close()
    assert(ok, message)
    return 1
end
