package "MysqlTest"

-- 测试 DML 写操作（INSERT / UPDATE / DELETE）返回的状态包及 affected_rows、last_insert_id
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_result(conn, err, result)
    conn.query_err = err
    conn.query_result = result
    conn.query_done = true
end

function test_dml()
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

    -- 1. 创建测试表
    conn.query_done = false
    conn:query("DROP TABLE IF EXISTS dml_test", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    conn.query_done = false
    conn:query("CREATE TABLE dml_test (id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(32), num INT)", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    -- 2. 插入多条记录，验证 status packet
    conn.query_done = false
    conn.query_err = nil
    conn.query_result = nil
    conn:query("INSERT INTO dml_test (name, num) VALUES ('row1', 10), ('row2', 20)", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    if conn.query_err ~= nil then
        if #conn.query_err > 0 then
            print("INSERT failed:", tostring(conn.query_err))
            conn:close()
            return 0
        end
    end

    local ins_res = conn.query_result
    if not ins_res then
        print("INSERT returned nil result")
        conn:close()
        return 0
    end

    -- 验证非结果集（result[1] == false）
    if ins_res[1] ~= false then
        print("expected status packet (false), got:", tostring(ins_res[1]))
        conn:close()
        return 0
    end

    -- 验证 affected_rows 为 2
    if ins_res[4] ~= 2 then
        print("expected affected_rows 2, got:", tostring(ins_res[4]))
        conn:close()
        return 0
    end

    -- 验证 last_insert_id >= 1
    if not ins_res[5] or ins_res[5] < 1 then
        print("expected valid last_insert_id, got:", tostring(ins_res[5]))
        conn:close()
        return 0
    end

    -- 3. UPDATE 操作
    conn.query_done = false
    conn.query_err = nil
    conn.query_result = nil
    conn:query("UPDATE dml_test SET num = 100 WHERE name = 'row1'", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    if conn.query_err ~= nil then
        if #conn.query_err > 0 then
            print("UPDATE failed:", tostring(conn.query_err))
            conn:close()
            return 0
        end
    end

    local upd_res = conn.query_result
    if not upd_res or upd_res[1] ~= false or upd_res[4] ~= 1 then
        print("UPDATE affected_rows expected 1, got:", upd_res and tostring(upd_res[4]) or "nil")
        conn:close()
        return 0
    end

    -- 4. DELETE 操作
    conn.query_done = false
    conn.query_err = nil
    conn.query_result = nil
    conn:query("DELETE FROM dml_test WHERE name = 'row2'", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    if conn.query_err ~= nil then
        if #conn.query_err > 0 then
            print("DELETE failed:", tostring(conn.query_err))
            conn:close()
            return 0
        end
    end

    local del_res = conn.query_result
    if not del_res or del_res[1] ~= false or del_res[4] ~= 1 then
        print("DELETE affected_rows expected 1, got:", del_res and tostring(del_res[4]) or "nil")
        conn:close()
        return 0
    end

    -- 清理
    conn.query_done = false
    conn:query("DROP TABLE IF EXISTS dml_test", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    conn:close()
    return 1
end
