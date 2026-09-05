package "MysqlTest"

-- 验证连接失败时错误信息中包含 "connect"/"refused"/"unable" 等关键字之一
function on_connect_msg(conn, err, success)
    conn.err = err
    conn.done = true
end

function test_error_message()
    local config = {}
    config["host"] = "127.0.0.1"
    config["port"] = 1
    config["user"] = "root"
    config["password"] = "irrelevant"
    config["db"] = "test"

    local conn = mysql.connect(config, "on_connect_msg")

    for i = 1, 200 do
        conn:tick()
        if conn.done then break end
    end

    if not conn.done then
        print("callback never fired")
        return 0
    end

    -- 错误信息应包含连接相关的关键字（不同实现可能返回不同文案）
    -- 旧实现："connect to host ... failed"  新实现（Boost.MySQL）："Connection refused"
    local got_err = tostring(conn.err or "")
    local ok = string.find(got_err, "connect")
        or string.find(got_err, "refus")
        or string.find(got_err, "unable")
        or string.find(got_err, "fail")
    if not ok then
        print("error message lacks connect/refus/unable/fail:", got_err)
        return 0
    end

    return 1
end
