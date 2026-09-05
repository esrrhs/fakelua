package "MysqlTest"

-- 测试基础数据类型及 NULL 字段映射为 nil
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_result(conn, err, result)
    conn.query_err = err
    conn.query_result = result
    conn.query_done = true
end

function test_datatypes()
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
    conn:query("DROP TABLE IF EXISTS dt_test", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    conn.query_done = false
    conn:query("CREATE TABLE dt_test (id INT PRIMARY KEY, val_null VARCHAR(32), val_float DOUBLE, val_date DATE, val_time TIME, val_str VARCHAR(64))", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    -- 2. 插入测试数据（包含 NULL、浮点数、日期、时间、字符串）
    conn.query_done = false
    conn:query("INSERT INTO dt_test VALUES (1, NULL, 3.1415, '2026-09-05', '14:30:00', 'hello_fakelua')", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    if conn.query_err ~= nil then
        if #conn.query_err > 0 then
            print("INSERT failed:", tostring(conn.query_err))
            conn:close()
            return 0
        end
    end

    -- 3. 查询并验证各类型字段映射
    conn.query_done = false
    conn.query_err = nil
    conn.query_result = nil
    conn:query("SELECT id, val_null, val_float, val_date, val_time, val_str FROM dt_test WHERE id = 1", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    if conn.query_err ~= nil then
        if #conn.query_err > 0 then
            print("SELECT failed:", tostring(conn.query_err))
            conn:close()
            return 0
        end
    end

    local result = conn.query_result
    if not result or result[1] ~= true or #result[3] ~= 1 then
        print("datatypes query result invalid")
        conn:close()
        return 0
    end

    local row = result[3][1]

    -- 验证 id 为 "1"
    if row[1] ~= "1" then
        print("id mismatch, expected '1', got:", tostring(row[1]))
        conn:close()
        return 0
    end

    -- 验证 NULL 列为 nil
    if row[2] ~= nil then
        print("val_null expected nil, got:", tostring(row[2]))
        conn:close()
        return 0
    end

    -- 验证 float 列
    if not row[3] or not string.find(tostring(row[3]), "3.1415") then
        print("val_float mismatch, got:", tostring(row[3]))
        conn:close()
        return 0
    end

    -- 验证 date 列
    if row[4] ~= "2026-09-05" then
        print("val_date mismatch, expected '2026-09-05', got:", tostring(row[4]))
        conn:close()
        return 0
    end

    -- 验证 time 列
    if not row[5] or not string.find(tostring(row[5]), "14:30:00") then
        print("val_time mismatch, got:", tostring(row[5]))
        conn:close()
        return 0
    end

    -- 验证 string 列
    if row[6] ~= "hello_fakelua" then
        print("val_str mismatch, expected 'hello_fakelua', got:", tostring(row[6]))
        conn:close()
        return 0
    end

    -- 清理
    conn.query_done = false
    conn:query("DROP TABLE IF EXISTS dt_test", "on_result")
    for i = 1, 1000 do conn:tick() if conn.query_done then break end end

    conn:close()
    return 1
end
