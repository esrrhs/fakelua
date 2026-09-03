package "MysqlTest"

-- 多结果集测试
local g_results = {}

function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_result(conn, err, result)
    conn.query_err = err
    conn.query_done = true
    local n = #g_results
    g_results[n + 1] = result
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
    g_results = {}
    conn.query_done = false
    conn.query_err = nil
    conn:query("SELECT 1 AS a; SELECT 2 AS b; SELECT 3 AS c", "on_result")

    -- 驱动足够长时间让所有结果返回
    for i = 1, 2000 do
        conn:tick()
        if conn.query_done and #g_results >= 3 then
            break
        end
    end

    -- 验证收到 3 个结果
    if #g_results < 3 then
        print("expected 3 results, got:", #g_results)
        conn:close()
        return 0
    end

    -- 验证每个结果
    for i = 1, #g_results do
        local result = g_results[i]
        if result[1] ~= true then
            print("result", i, "is not a result set")
            conn:close()
            return 0
        end
        local expected = tostring(i)
        if result[3][1][1] ~= expected then
            print("result", i, "value mismatch:", result[3][1][1], "expected:", expected)
            conn:close()
            return 0
        end
    end

    conn:close()
    return 1
end
