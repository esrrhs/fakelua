package "MysqlTest"

-- 集成测试：回调式 API
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_result(conn, err, result)
    conn.query_err = err
    if result then
        conn.result_is_result_set = result[1] == true
        if result[3] and result[3][1] and result[3][1][1] then
            conn.result_first_value = result[3][1][1]
        else
            conn.result_first_value = nil
        end
    else
        conn.result_is_result_set = false
        conn.result_first_value = nil
    end
    conn.query_done = true
end

function test_mysql_integration()
    -- Try with password first (Linux CI), then without (Windows/macOS)
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
        -- Try without password
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

    conn.query_done = false
    conn.query_err = nil
    conn.result_is_result_set = nil
    conn.result_first_value = nil
    conn:query("SELECT 1 AS test", "on_result")

    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    if type(conn.query_err) == "string" then
        if #conn.query_err > 0 then
            local err_str = conn.query_err
            print("query failed:", tostring(err_str))
            conn:close()
            return 0
        end
    end

    if conn.result_is_result_set ~= true or conn.result_first_value ~= "1" then
        print("query result mismatch:", conn.result_first_value)
        conn:close()
        return 0
    end

    conn:close()
    return 1
end
