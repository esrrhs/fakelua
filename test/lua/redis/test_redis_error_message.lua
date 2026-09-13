package "RedisTest"

-- 验证连接失败时错误信息中包含 "connect"/"refus"/"unable"/"fail" 等关键字之一
function on_connect_msg(conn, err, success)
    conn.err = err
    conn.done = true
end

function test_error_message()
    local config = {}
    config["host"] = "127.0.0.1"
    config["port"] = 1
    config["timeout_ms"] = 1000

    local conn = redis.connect(config, "on_connect_msg")

    for i = 1, 1500 do
        runtime.tick()
        if conn.done then break end
        os.sleep(1)
    end

    if not conn.done then
        print("callback never fired")
        return 0
    end

    local got_err = tostring(conn.err or "")
    local ok = string.find(got_err, "connect")
        or string.find(got_err, "refus")
        or string.find(got_err, "unable")
        or string.find(got_err, "fail")
        or string.find(got_err, "reset")
        or string.find(got_err, "timeout")
    if not ok then
        print("error message lacks connect/refus/unable/fail:", got_err)
        return 0
    end

    return 1
end
