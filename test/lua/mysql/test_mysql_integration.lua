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
        -- 没有 MySQL 服务或连接失败时跳过
        print("skipping: cannot connect to MySQL (", tostring(conn.connect_err or ""), ")")
        return 1  -- skip
    end

    -- 1. 创建表
    conn.query_done = false
    conn.query_err = nil
    conn:query("DROP TABLE IF EXISTS fakelua_test", "on_result")

    for i = 1, 200 do conn:tick() if conn.query_done then break end end
    if conn.query_err and #conn.query_err > 0 then
        print("DROP TABLE failed:", conn.query_err)
        conn:close()
        return 0
    end

    -- 2. CREATE TABLE
    conn.query_done = false
    conn.query_err = nil
    conn:query("CREATE TABLE fakelua_test (id INT PRIMARY KEY, name VARCHAR(64), val FLOAT)", "on_result")

    for i = 1, 200 do conn:tick() if conn.query_done then break end end
    if conn.query_err and #conn.query_err > 0 then
        print("CREATE TABLE failed:", conn.query_err)
        conn:close()
        return 0
    end

    -- 3. INSERT 插入数据
    conn.query_done = false
    conn.query_err = nil
    conn:query("INSERT INTO fakelua_test VALUES (1, 'alice', 1.5)", "on_result")

    for i = 1, 200 do conn:tick() if conn.query_done then break end end
    if conn.query_err and #conn.query_err > 0 then
        print("INSERT failed:", conn.query_err)
        conn:close()
        return 0
    end

    -- 再插两条
    conn.query_done = false
    conn:query("INSERT INTO fakelua_test VALUES (2, 'bob', 2.7)", "on_result")
    for i = 1, 200 do conn:tick() if conn.query_done then break end end

    conn.query_done = false
    conn:query("INSERT INTO fakelua_test VALUES (3, NULL, 3.14)", "on_result")
    for i = 1, 200 do conn:tick() if conn.query_done then break end end

    -- 4. SELECT 查询并验证结果集
    conn.query_done = false
    conn.query_err = nil
    conn:query("SELECT id, name, val FROM fakelua_test ORDER BY id", "on_result")

    for i = 1, 200 do conn:tick() if conn.query_done then break end end
    if conn.query_err and #conn.query_err > 0 then
        print("SELECT failed:", conn.query_err)
        conn:close()
        return 0
    end

    local result = conn.query_result
    if not result then
        print("SELECT returned nil result")
        conn:close()
        return 0
    end

    -- 验证是结果集
    if result[1] ~= true then
        print("expected result set (is_result_set=true), got:", result[1])
        conn:close()
        return 0
    end

    -- 验证列
    if #result[2] ~= 3 then
        print("expected 3 columns, got:", #result[2])
        conn:close()
        return 0
    end
    if result[2][1][1] ~= "id" then
        print("column 1 name mismatch:", result[2][1][1])
        conn:close()
        return 0
    end
    if result[2][2][1] ~= "name" then
        print("column 2 name mismatch:", result[2][2][1])
        conn:close()
        return 0
    end

    -- 验证行数
    if #result[3] ~= 3 then
        print("expected 3 rows, got:", #result[3])
        conn:close()
        return 0
    end

    -- 验证第一行: 1, "alice", "1.5"
    local row1 = result[3][1]
    if row1[1] ~= "1" or row1[2] ~= "alice" then
        print("row 1 mismatch:", row1[1], row1[2])
        conn:close()
        return 0
    end

    -- 验证 NULL 映射为 nil（第三行 name = NULL）
    local row3 = result[3][3]
    if row3[2] ~= nil then
        print("expected nil for NULL name, got:", row3[2])
        conn:close()
        return 0
    end

    -- 5. UPDATE
    conn.query_done = false
    conn.query_err = nil
    conn:query("UPDATE fakelua_test SET name='charlie' WHERE id=2", "on_result")

    for i = 1, 200 do conn:tick() if conn.query_done then break end end
    if conn.query_err and #conn.query_err > 0 then
        print("UPDATE failed:", conn.query_err)
        conn:close()
        return 0
    end
    -- 验证 affected_rows
    local update_result = conn.query_result
    if update_result[4] ~= 1 then
        print("UPDATE affected_rows expected 1, got:", update_result[4])
        conn:close()
        return 0
    end

    -- 6. DELETE
    conn.query_done = false
    conn.query_err = nil
    conn:query("DELETE FROM fakelua_test WHERE id=3", "on_result")

    for i = 1, 200 do conn:tick() if conn.query_done then break end end
    if conn.query_err and #conn.query_err > 0 then
        print("DELETE failed:", conn.query_err)
        conn:close()
        return 0
    end
    local delete_result = conn.query_result
    if delete_result[4] ~= 1 then
        print("DELETE affected_rows expected 1, got:", delete_result[4])
        conn:close()
        return 0
    end

    -- 7. 验证删除后只剩 2 行
    conn.query_done = false
    conn:query("SELECT COUNT(*) FROM fakelua_test", "on_result")
    for i = 1, 200 do conn:tick() if conn.query_done then break end end
    local count_result = conn.query_result
    if count_result[3][1][1] ~= "2" then
        print("expected 2 rows after delete, got:", count_result[3][1][1])
        conn:close()
        return 0
    end

    -- 清理
    conn:query("DROP TABLE IF EXISTS fakelua_test", "on_result")
    for i = 1, 200 do conn:tick() if conn.query_done then break end end
    conn:close()
    return 1
end
