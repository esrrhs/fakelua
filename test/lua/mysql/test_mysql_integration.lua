package "MysqlTest"

-- 集成测试：需要本地 MySQL 服务（端口 3306，用户 root/root，数据库 test）
-- 测试连接、查询、插入、结果集解析
function test_mysql_integration()
    local conn = mysql.connect({
        host = "127.0.0.1",
        port = 3306,
        user = "root",
        password = "root",
        db = "test",
    })
    if not conn then
        print("failed to connect to MySQL")
        return 0
    end

    -- 创建测试表
    local ok, err = pcall(function()
        conn:query("DROP TABLE IF EXISTS fakelua_test")
        conn:query("CREATE TABLE fakelua_test (id INT PRIMARY KEY, name VARCHAR(64), val FLOAT)")
    end)
    if not ok then
        print("failed to create table:", tostring(err))
        conn:close()
        return 0
    end

    -- 插入数据
    ok, err = pcall(function()
        conn:query("INSERT INTO fakelua_test VALUES (1, 'alice', 1.5)")
        conn:query("INSERT INTO fakelua_test VALUES (2, 'bob', 2.7)")
        conn:query("INSERT INTO fakelua_test VALUES (3, 'charlie', 3.14)")
    end)
    if not ok then
        print("failed to insert:", tostring(err))
        conn:close()
        return 0
    end

    -- 查询并验证结果集
    local result = conn:query("SELECT id, name, val FROM fakelua_test ORDER BY id")
    if not result then
        print("query returned nil")
        conn:close()
        return 0
    end

    if not result.is_result_set then
        print("expected result set, got:", type(result))
        conn:close()
        return 0
    end

    -- 验证列
    if #result.columns ~= 3 then
        print("expected 3 columns, got:", #result.columns)
        conn:close()
        return 0
    end

    -- 验证行数
    if #result.rows ~= 3 then
        print("expected 3 rows, got:", #result.rows)
        conn:close()
        return 0
    end

    -- 验证第一行数据
    local row1 = result.rows[1]
    if row1[1] ~= "1" or row1[2] ~= "alice" then
        print("row 1 mismatch:", row1[1], row1[2])
        conn:close()
        return 0
    end

    -- 清理
    conn:query("DROP TABLE IF EXISTS fakelua_test")
    conn:close()
    return 1
end

-- 测试 NULL 值映射为 nil
function test_mysql_null()
    local conn = mysql.connect({
        host = "127.0.0.1",
        port = 3306,
        user = "root",
        password = "root",
        db = "test",
    })
    if not conn then
        print("failed to connect to MySQL")
        return 0
    end

    local ok, err = pcall(function()
        conn:query("DROP TABLE IF EXISTS fakelua_null_test")
        conn:query("CREATE TABLE fakelua_null_test (id INT PRIMARY KEY, name VARCHAR(64))")
        conn:query("INSERT INTO fakelua_null_test VALUES (1, NULL)")
    end)
    if not ok then
        print("failed to setup:", tostring(err))
        conn:close()
        return 0
    end

    local result = conn:query("SELECT id, name FROM fakelua_null_test WHERE id = 1")
    if not result or not result.is_result_set then
        print("expected result set")
        conn:close()
        return 0
    end

    if #result.rows ~= 1 then
        print("expected 1 row, got:", #result.rows)
        conn:close()
        return 0
    end

    -- NULL 应映射为 nil
    local row = result.rows[1]
    if row[1] ~= "1" then
        print("id mismatch:", row[1])
        conn:close()
        return 0
    end
    if row[2] ~= nil then
        print("expected nil for NULL, got:", row[2])
        conn:close()
        return 0
    end

    conn:query("DROP TABLE IF EXISTS fakelua_null_test")
    conn:close()
    return 1
end
