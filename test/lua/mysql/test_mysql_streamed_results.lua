package "MysqlStream"

function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_error = err
end

function on_result(conn, err, result, label)
    conn.result_error = err
    local count = conn.callback_count + 1
    conn.callback_count = count
    conn.results[count] = result
    conn.labels[count] = label
end

local function connect_with_password(password)
    local conn = mysql.connect({host = "127.0.0.1", port = 3306, user = "root", password = password, db = "test"}, "MysqlStream.on_connect")
    for i = 1, 5000 do
        runtime.tick()
        if conn.connected or conn.connect_error then break end
        os.sleep(1)
    end
    return conn
end

local function wait_results(conn, count)
    for i = 1, 5000 do
        runtime.tick()
        if conn.result_error or conn.callback_count >= count then break end
        os.sleep(1)
    end
    assert(conn.result_error == nil, conn.result_error)
    assert(conn.callback_count == count)
end

function test_large_and_queued_results()
    local conn = connect_with_password("root")
    if not conn.connected then
        conn:close()
        conn = connect_with_password("")
    end
    assert(conn.connected, conn.connect_error)
    local ok, err = pcall(function()
        conn.callback_count = 0
        conn.results = {}
        conn.labels = {}
        conn:query("SELECT REPEAT('x', 262144) AS payload; SELECT SLEEP(0.02) AS waited; SELECT 7 AS value", "MysqlStream.on_result", "multi")
        wait_results(conn, 3)
        assert(#conn.results[1][3][1][1] == 262144)
        assert(string.sub(conn.results[1][3][1][1], 1, 3) == "xxx")
        assert(conn.results[2][3][1][1] == 0 and conn.results[3][3][1][1] == 7)
        assert(conn.labels[1] == "multi" and conn.labels[3] == "multi")
        conn.callback_count = 0
        conn.results = {}
        conn.labels = {}
        conn:query("SELECT SLEEP(0.02) AS waited", "MysqlStream.on_result", "first")
        conn:query("SELECT 77 AS value", "MysqlStream.on_result", "second")
        wait_results(conn, 2)
        assert(conn.labels[1] == "first" and conn.labels[2] == "second", "queued callback labels: " .. tostring(conn.labels[1]) .. ", " .. tostring(conn.labels[2]))
        assert(conn.results[1][3][1][1] == 0 and conn.results[2][3][1][1] == 77)
    end)
    conn:close()
    assert(ok, err)
    return 1
end

local function reset_result(conn)
    conn.callback_count = 0
    conn.result_error = nil
    conn.results = {}
    conn.labels = {}
end

local function wait_error(conn, text)
    for i = 1, 5000 do
        runtime.tick()
        if conn.callback_count > 0 then break end
        os.sleep(1)
    end
    assert(conn.callback_count == 1 and type(conn.result_error) == "string")
    if text then assert(string.find(string.lower(conn.result_error), text, 1, true)) end
end

function test_statement_and_late_multi_result_errors()
    local conn = connect_with_password("root")
    if not conn.connected then
        conn:close()
        conn = connect_with_password("")
    end
    assert(conn.connected, conn.connect_error)
    local ok, err = pcall(function()
        reset_result(conn)
        conn:stmt_prepare("THIS IS NOT SQL", "MysqlStream.on_result", "bad-prepare")
        wait_error(conn, nil)
        assert(conn.labels[1] == "bad-prepare")
        reset_result(conn)
        conn:query("CREATE TEMPORARY TABLE fakelua_module_stmt(value INT UNIQUE)", "MysqlStream.on_result", "create")
        wait_results(conn, 1)
        reset_result(conn)
        conn:stmt_prepare("INSERT INTO fakelua_module_stmt VALUES(?)", "MysqlStream.on_result", "prepare")
        wait_results(conn, 1)
        local statement = conn.results[1]
        assert(type(statement) == "number" and statement > 0)
        reset_result(conn)
        conn:stmt_execute(statement, {7}, "MysqlStream.on_result", "insert")
        wait_results(conn, 1)
        reset_result(conn)
        conn:stmt_execute(statement, {7}, "MysqlStream.on_result", "duplicate")
        wait_error(conn, "duplicate")
        assert(conn.labels[1] == "duplicate")
        conn:stmt_close(statement)
        reset_result(conn)
        conn:stmt_execute(statement, {8}, "MysqlStream.on_result", "closed-statement")
        wait_error(conn, "not prepared")
        reset_result(conn)
        conn:query("SELECT 1; SELECT missing_column FROM fakelua_module_stmt", "MysqlStream.on_result", "late-error")
        wait_error(conn, nil)
        assert(conn.labels[1] == "late-error")
        reset_result(conn)
        conn:query("SELECT 9", "MysqlStream.on_result", "recovered")
        wait_results(conn, 1)
        assert(conn.results[1][3][1][1] == 9)
    end)
    conn:close()
    assert(ok, err)
    return 1
end

function test_row_stream_continuations()
    local conn = connect_with_password("root")
    if not conn.connected then conn:close() conn = connect_with_password("") end
    assert(conn.connected, conn.connect_error)
    local ok, message = pcall(function()
        reset_result(conn)
        local source = "(SELECT 0 AS n UNION ALL SELECT 1)"
        local sql = "SELECT REPEAT('x',1024) AS payload, SLEEP(0.002) AS waited FROM " .. source .. " AS a"
        for _, alias in ipairs({"b", "c", "d", "e", "f"}) do sql = sql .. " CROSS JOIN " .. source .. " AS " .. alias end
        conn:query(sql .. "; SELECT 9 AS final_value", "MysqlStream.on_result", "row-stream")
        wait_results(conn, 2)
        local rows = conn.results[1][3]
        assert(#rows == 64)
        for i = 1, #rows do assert(#rows[i][1] == 1024 and rows[i][2] == 0) end
        assert(conn.results[2][3][1][1] == 9)
        assert(conn.labels[1] == "row-stream" and conn.labels[2] == "row-stream")
    end)
    conn:close()
    assert(ok, message)
    return 1
end


function test_configuration_and_statement_parameter_contracts()
    assert(not pcall(function() mysql.connect(false, "MysqlStream.on_connect") end))
    assert(not pcall(function() mysql.connect({user = "root", timeout_ms = -1}, "MysqlStream.on_connect") end))
    assert(not pcall(function() mysql.connect({user = "root", timeout_ms = 2147483648}, "MysqlStream.on_connect") end))
    for _, mode in ipairs({0, 1, "require", "enable", "disable"}) do
        local unused = mysql.connect({host = "127.0.0.1", user = "root", ssl = mode}, "MysqlStream.on_connect")
        unused:close()
    end
    local conn = connect_with_password("root")
    if not conn.connected then conn:close() conn = connect_with_password("") end
    assert(conn.connected, conn.connect_error)
    local ok, message = pcall(function()
        reset_result(conn)
        conn:stmt_prepare("SELECT CAST(? AS SIGNED) AS flag, ? AS absent", "MysqlStream.on_result", "prepare")
        wait_results(conn, 1)
        local statement = conn.results[1]
        reset_result(conn)
        conn:stmt_execute(statement, {true, n = 2}, "MysqlStream.on_result", "bool-nil")
        wait_results(conn, 1)
        assert(conn.results[1][3][1][1] == 1 and conn.results[1][3][1][2] == nil)
        assert(not pcall(function() conn:stmt_execute(statement, {n = 65536}, "MysqlStream.on_result") end))
        reset_result(conn)
        conn:stmt_execute(statement, {true}, "MysqlStream.on_result", "mismatch")
        wait_error(conn, "count")
        conn:stmt_close(statement)
        reset_result(conn)
        local source = "(SELECT 0 AS n UNION ALL SELECT 1)"
        local sql = "SELECT REPEAT('s',1024), SLEEP(0.002) FROM " .. source .. " AS a"
        for _, alias in ipairs({"b", "c", "d", "e", "f"}) do sql = sql .. " CROSS JOIN " .. source .. " AS " .. alias end
        conn:query(sql, "MysqlStream.on_result", "single-stream")
        wait_results(conn, 1)
        assert(#conn.results[1][3] == 64)
    end)
    conn:close()
    assert(ok, message)
    return 1
end
