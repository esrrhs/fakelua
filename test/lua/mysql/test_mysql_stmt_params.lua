package "MysqlTest"

-- 测试预处理语句 NULL 参数绑定、多种数据类型绑定及语句复用
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

function test_stmt_params()
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
    conn:query("DROP TABLE IF EXISTS stmt_p_test", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    conn.query_done = false
    conn:query("CREATE TABLE stmt_p_test (id INT PRIMARY KEY, name VARCHAR(64), score DOUBLE)", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    -- 2. Prepare INSERT
    conn.prepare_done = false
    conn.stmt_id = nil
    conn:stmt_prepare("INSERT INTO stmt_p_test VALUES (?, ?, ?)", "on_prepare")
    for i = 1, 1000 do conn:tick() if conn.prepare_done then break end end

    if not conn.stmt_id then
        print("stmt_prepare INSERT failed:", tostring(conn.prepare_err))
        conn:close()
        return 0
    end

    local insert_stmt_id = conn.stmt_id

    -- 3. 执行第一次 INSERT：带有 nil (NULL) 与浮点数
    conn.query_done = false
    conn.query_err = nil
    conn:stmt_execute(insert_stmt_id, {1, nil, 99.5}, "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    if conn.query_err ~= nil then
        if #conn.query_err > 0 then
            print("stmt_execute with nil failed:", tostring(conn.query_err))
            conn:stmt_close(insert_stmt_id)
            conn:close()
            return 0
        end
    end

    -- 4. 复用同一个 stmt_id 执行第二次 INSERT
    conn.query_done = false
    conn.query_err = nil
    conn:stmt_execute(insert_stmt_id, {2, "bob", 88.0}, "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    if conn.query_err ~= nil then
        if #conn.query_err > 0 then
            print("stmt_execute reuse failed:", tostring(conn.query_err))
            conn:stmt_close(insert_stmt_id)
            conn:close()
            return 0
        end
    end

    conn:stmt_close(insert_stmt_id)

    -- 5. Prepare SELECT
    conn.prepare_done = false
    conn.stmt_id = nil
    conn:stmt_prepare("SELECT id, name, score FROM stmt_p_test WHERE id = ?", "on_prepare")
    for i = 1, 1000 do conn:tick() if conn.prepare_done then break end end

    if not conn.stmt_id then
        print("stmt_prepare SELECT failed:", tostring(conn.prepare_err))
        conn:close()
        return 0
    end

    local select_stmt_id = conn.stmt_id

    -- 查询 id = 1（验证第 2 列确实存入了 NULL，在 Lua 中得到 nil）
    conn.query_done = false
    conn.query_err = nil
    conn:stmt_execute(select_stmt_id, {1}, "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    local res1 = conn.query_result
    if not res1 or res1[1] ~= true or #res1[3] ~= 1 then
        print("SELECT id=1 result invalid")
        conn:stmt_close(select_stmt_id)
        conn:close()
        return 0
    end

    local r1 = res1[3][1]
    if r1[1] ~= "1" then
        print("r1 id expected '1', got:", tostring(r1[1]))
        conn:stmt_close(select_stmt_id)
        conn:close()
        return 0
    end
    if r1[2] ~= nil then
        print("r1 name expected nil (NULL), got:", tostring(r1[2]))
        conn:stmt_close(select_stmt_id)
        conn:close()
        return 0
    end
    if not r1[3] or not string.find(tostring(r1[3]), "99.5") then
        print("r1 score mismatch:", tostring(r1[3]))
        conn:stmt_close(select_stmt_id)
        conn:close()
        return 0
    end

    -- 查询 id = 2（验证字符串和数值）
    conn.query_done = false
    conn.query_err = nil
    conn:stmt_execute(select_stmt_id, {2}, "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    local res2 = conn.query_result
    if not res2 or res2[1] ~= true or #res2[3] ~= 1 then
        print("SELECT id=2 result invalid")
        conn:stmt_close(select_stmt_id)
        conn:close()
        return 0
    end

    local r2 = res2[3][1]
    if r2[1] ~= "2" or r2[2] ~= "bob" then
        print("r2 values mismatch:", tostring(r2[1]), tostring(r2[2]))
        conn:stmt_close(select_stmt_id)
        conn:close()
        return 0
    end

    -- 清理
    conn:stmt_close(select_stmt_id)
    conn.query_done = false
    conn:query("DROP TABLE IF EXISTS stmt_p_test", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    conn:close()
    return 1
end
