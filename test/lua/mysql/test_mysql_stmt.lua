package "MysqlTest"

-- 预处理语句测试
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_result(conn, err, result)
    conn.query_err = err
    conn.query_result = result
    conn.query_done = true
end

function on_prepare(conn, err, stmt_id)
    conn.stmt_id = stmt_id
    conn.prepare_err = err
    conn.prepare_done = true
end

function test_stmt()
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

    -- 创建测试表
    conn.query_done = false
    conn.query_err = nil
    conn:query("DROP TABLE IF EXISTS stmt_test", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    conn.query_done = false
    conn:query("CREATE TABLE stmt_test (id INT PRIMARY KEY, name VARCHAR(64))", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    -- 准备 INSERT 语句
    conn.prepare_done = false
    conn.prepare_err = nil
    conn.stmt_id = nil
    conn:stmt_prepare("INSERT INTO stmt_test VALUES (?, ?)", "on_prepare")

    for i = 1, 1000 do
        conn:tick()
        if conn.prepare_done then break end
    end

    if not conn.stmt_id then
        local err_str = conn.prepare_err or ""
        print("stmt_prepare failed:", tostring(err_str))
        conn:close()
        return 0
    end

    -- 执行插入
    conn.query_done = false
    conn.query_err = nil
    conn:stmt_execute(conn.stmt_id, {"1", "alice"}, "on_result")

    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    if conn.query_err and #conn.query_err > 0 then
        local err_str = conn.query_err
        print("stmt_execute failed:", tostring(err_str))
        conn:stmt_close(conn.stmt_id)
        conn:close()
        return 0
    end

    -- 验证插入成功
    local insert_result = conn.query_result
    if not insert_result or insert_result[4] ~= 1 then
        print("INSERT affected_rows expected 1, got:", insert_result and insert_result[4] or "nil")
        conn:stmt_close(conn.stmt_id)
        conn:close()
        return 0
    end

    -- 准备 SELECT 语句
    conn.prepare_done = false
    conn.stmt_id = nil
    conn:stmt_prepare("SELECT id, name FROM stmt_test WHERE id = ?", "on_prepare")

    for i = 1, 1000 do
        conn:tick()
        if conn.prepare_done then break end
    end

    if not conn.stmt_id then
        local err_str = conn.prepare_err or ""
        print("SELECT stmt_prepare failed:", tostring(err_str))
        conn:close()
        return 0
    end

    -- 执行查询
    conn.query_done = false
    conn.query_err = nil
    conn:stmt_execute(conn.stmt_id, {"1"}, "on_result")

    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    if conn.query_err and #conn.query_err > 0 then
        local err_str = conn.query_err
        print("SELECT stmt_execute failed:", tostring(err_str))
        conn:stmt_close(conn.stmt_id)
        conn:close()
        return 0
    end

    -- 验证查询结果
    local result = conn.query_result
    if not result then
        print("SELECT returned nil")
        conn:stmt_close(conn.stmt_id)
        conn:close()
        return 0
    end

    if result[1] ~= true then
        print("expected result set, got:", result[1])
        conn:stmt_close(conn.stmt_id)
        conn:close()
        return 0
    end

    if #result[3] ~= 1 then
        print("expected 1 row, got:", #result[3])
        conn:stmt_close(conn.stmt_id)
        conn:close()
        return 0
    end

    local row = result[3][1]
    if row[1] ~= "1" or row[2] ~= "alice" then
        print("row mismatch:", row[1], row[2])
        conn:stmt_close(conn.stmt_id)
        conn:close()
        return 0
    end

    -- 清理
    conn:stmt_close(conn.stmt_id)
    conn:query("DROP TABLE IF EXISTS stmt_test", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end
    conn:close()
    return 1
end
