package "MysqlTest"

-- 集成测试：回调式 API（需要本地 MySQL 服务）
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_result(conn, err, result)
    conn.query_err = err
    conn.query_result = result
    conn.query_done = true
end

function test_mysql_integration()
    local config = {}
    config["host"] = "127.0.0.1"
    config["port"] = 3306
    config["user"] = "root"
    config["password"] = "root"
    config["db"] = "test"

    local conn = mysql.connect(config, "on_connect")

    -- 驱动 IO 直到连接完成（最多 200 次 tick）
    for i = 1, 200 do
        conn:tick()
        if conn.connected or conn.connect_err then break end
    end

    if not conn.connected then
        -- 没有 MySQL 服务时跳过（CI 会启动 MySQL）
        local err_str = tostring(conn.connect_err or "")
        if string.find(err_str, "connection closed") or string.find(err_str, "connect failed") then
            print("skipping: no MySQL server (", err_str, ")")
            return 1  -- skip
        end
        print("failed to connect:", err_str)
        return 0
    end

    -- 创建表
    conn.query_done = false
    conn.query_err = nil
    conn:query("DROP TABLE IF EXISTS fakelua_test", "on_result")

    for i = 1, 200 do
        conn:tick()
        if conn.query_done then break end
    end

    if conn.query_err and #conn.query_err > 0 then
        print("failed to drop table:", conn.query_err)
        conn:close()
        return 0
    end

    -- 清理
    conn:close()
    return 1
end
