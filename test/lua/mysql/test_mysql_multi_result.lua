package "MysqlTest"

-- 多结果集测试
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_result(conn, err, result)
    conn.query_err = err
    local n = conn.result_count or 0
    n = n + 1
    conn.result_count = n
    if result and result[3] and result[3][1] then
        if n == 1 then
            conn.result1_value = result[3][1][1]
        elseif n == 2 then
            conn.result2_value = result[3][1][1]
        elseif n == 3 then
            conn.result3_value = result[3][1][1]
        end
    end
    conn.query_done = true
end

function test_multi_result()
    -- Try with password first, then without
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

    -- 多语句查询
    conn.query_done = false
    conn.query_err = nil
    conn.result_count = 0
    conn.result1_value = nil
    conn.result2_value = nil
    conn.result3_value = nil
    conn:query("SELECT 1 AS a; SELECT 2 AS b; SELECT 3 AS c", "on_result")

    -- 驱动足够长时间让所有结果返回
    for i = 1, 2000 do
        conn:tick()
        if conn.query_done and (conn.result_count or 0) >= 3 then
            break
        end
    end

    -- 验证收到 3 个结果
    if (conn.result_count or 0) < 3 then
        print("expected 3 results, got:", conn.result_count)
        conn:close()
        return 0
    end

    if conn.result1_value ~= "1" then
        print("result", 1, "value mismatch:", conn.result1_value, "expected:", "1")
        conn:close()
        return 0
    end
    if conn.result2_value ~= "2" then
        print("result", 2, "value mismatch:", conn.result2_value, "expected:", "2")
        conn:close()
        return 0
    end
    if conn.result3_value ~= "3" then
        print("result", 3, "value mismatch:", conn.result3_value, "expected:", "3")
        conn:close()
        return 0
    end

    conn:close()
    return 1
end
