package "SqliteTest"

-- Each test uses a unique db file to avoid conflicts

function test_open_memory()
    local db = sqlite.open("/tmp/fakelua_test_open.db")
    if type(db) ~= "table" then return 0 end
    db:close()
    return 1
end

function test_create_table()
    local db = sqlite.open("/tmp/fakelua_test_create.db")
    local result = db:exec("CREATE TABLE users (id INTEGER, name TEXT)")
    if result ~= nil then return 0 end
    db:close()
    return 1
end

function test_insert()
    local db = sqlite.open("/tmp/fakelua_test_insert.db")
    db:exec("CREATE TABLE t (id INTEGER, val TEXT)")
    local result = db:exec("INSERT INTO t VALUES (1, 'hello')")
    if result ~= nil then return 0 end
    db:close()
    return 1
end

function test_select()
    local db = sqlite.open("/tmp/fakelua_test_select.db")
    db:exec("CREATE TABLE t (id INTEGER, name TEXT)")
    db:exec("INSERT INTO t VALUES (1, 'Alice')")
    db:exec("INSERT INTO t VALUES (2, 'Bob')")
    local rows = db:exec("SELECT * FROM t")
    if type(rows) ~= "table" then return 0 end
    if #rows ~= 2 then return 0 end
    if rows[1].id ~= 1 then return 0 end
    if rows[1].name ~= "Alice" then return 0 end
    if rows[2].id ~= 2 then return 0 end
    if rows[2].name ~= "Bob" then return 0 end
    db:close()
    return 1
end

function test_select_empty()
    local db = sqlite.open("/tmp/fakelua_test_empty.db")
    db:exec("CREATE TABLE t (id INTEGER)")
    local rows = db:exec("SELECT * FROM t")
    if type(rows) ~= "table" then return 0 end
    if #rows ~= 0 then return 0 end
    db:close()
    return 1
end

function test_insert_return_nil()
    local db = sqlite.open("/tmp/fakelua_test_nil.db")
    db:exec("CREATE TABLE t (id INTEGER)")
    local result = db:exec("INSERT INTO t VALUES (42)")
    if result ~= nil then return 0 end
    db:close()
    return 1
end

function test_close()
    local db = sqlite.open("/tmp/fakelua_test_close.db")
    db:close()
    return 1
end

function test_multiple_inserts()
    local db = sqlite.open("/tmp/fakelua_test_multi.db")
    db:exec("CREATE TABLE t (a INTEGER, b REAL, c TEXT)")
    db:exec("INSERT INTO t VALUES (1, 1.5, 'one')")
    db:exec("INSERT INTO t VALUES (2, 2.5, 'two')")
    db:exec("INSERT INTO t VALUES (3, 3.5, 'three')")
    local rows = db:exec("SELECT * FROM t")
    if #rows ~= 3 then return 0 end
    if rows[1].a ~= 1 then return 0 end
    if rows[2].b < 2.4 or rows[2].b > 2.6 then return 0 end
    if rows[3].c ~= "three" then return 0 end
    db:close()
    return 1
end

function test_select_where()
    local db = sqlite.open("/tmp/fakelua_test_where.db")
    db:exec("CREATE TABLE t (id INTEGER, name TEXT)")
    db:exec("INSERT INTO t VALUES (1, 'Alice')")
    db:exec("INSERT INTO t VALUES (2, 'Bob')")
    db:exec("INSERT INTO t VALUES (3, 'Charlie')")
    local rows = db:exec("SELECT * FROM t WHERE id > 1")
    if #rows ~= 2 then return 0 end
    if rows[1].name ~= "Bob" then return 0 end
    if rows[2].name ~= "Charlie" then return 0 end
    db:close()
    return 1
end

function test_auto_close()
    local db = sqlite.open("/tmp/fakelua_test_auto.db")
    db:exec("CREATE TABLE t (id INTEGER)")
    db:exec("INSERT INTO t VALUES (1)")
    -- do not close explicitly; finalizer should handle it
    return 1
end
