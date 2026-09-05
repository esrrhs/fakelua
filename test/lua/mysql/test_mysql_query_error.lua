package "MysqlTest"

-- 测试 SQL 语法错误/表不存在错误及连接恢复能力
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_result(conn, err, result)
    conn.query_err = err
    conn.query_result = result
    conn.query_done = true
end

function test_query_error()
    local conn = mysql.connect({
        host = "127.0.0.1",
        port = 3306,
        user = "root",
        password = "root",
        db = "test"
    }, "on_connect")

    for i = 1, 1000 do
        conn:tick()
        if conn.connected or conn.connect_err then break end
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
            conn:tick()
            if conn.connected or conn.connect_err then break end
        end
    end

    if not conn.connected then
        local err_str = conn.connect_err or ""
        print("failed to connect:", tostring(err_str))
        return 0
    end

    -- 1. 执行有错误的 SQL（查询一个绝对不存在的表）
    conn.query_done = false
    conn.query_err = nil
    conn.query_result = nil
    conn:query("SELECT * FROM non_existent_table_12345_fakelua", "on_result")

    for i = 1, 1000 do
        conn:tick()
        if conn.query_done then break end
    end

    if not conn.query_done then
        print("query_error: callback never fired")
        conn:close()
        return 0
    end

    -- 期望收到错误信息
    if conn.query_err == nil then
        print("expected error for non-existent table, but got nil")
        conn:close()
        return 0
    end

    local err_msg = tostring(conn.query_err)
    if #err_msg == 0 then
        print("query_error message is empty")
        conn:close()
        return 0
    end

    -- 2. 关键验证：出错后连接状态恢复，依然能够正常执行后续正常查询
    conn.query_done = false
    conn.query_err = nil
    conn.query_result = nil
    conn:query("SELECT 42 AS recovered", "on_result")

    for i = 1, 1000 do
        conn:tick()
        if conn.query_done then break end
    end

    if not conn.query_done then
        print("recovery query: callback never fired")
        conn:close()
        return 0
    end

    if conn.query_err ~= nil then
        if #conn.query_err > 0 then
            print("recovery query failed:", tostring(conn.query_err))
            conn:close()
            return 0
        end
    end

    local res = conn.query_result
    if not res or res[1] ~= true or res[3][1][1] ~= "42" then
        print("recovery query result mismatch")
        conn:close()
        return 0
    end

    conn:close()
    return 1
end
