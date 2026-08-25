package "SqliteTest"

-- Use in-memory databases so tests are portable (Windows has no /tmp).

local function open_db()
    return sqlite.open(":memory:")
end

function test_open_memory()
    local db = open_db()
    if type(db) ~= "table" then return 0 end
    db:close()
    return 1
end

function test_create_table()
    local db = open_db()
    local result = db:exec("CREATE TABLE users (id INTEGER, name TEXT)")
    if result ~= nil then return 0 end
    db:close()
    return 1
end

function test_insert()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER, val TEXT)")
    local result = db:exec("INSERT INTO t VALUES (1, 'hello')")
    if result ~= nil then return 0 end
    db:close()
    return 1
end

function test_select()
    local db = open_db()
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
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER)")
    local rows = db:exec("SELECT * FROM t")
    if type(rows) ~= "table" then return 0 end
    if #rows ~= 0 then return 0 end
    db:close()
    return 1
end

function test_insert_return_nil()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER)")
    local result = db:exec("INSERT INTO t VALUES (42)")
    if result ~= nil then return 0 end
    db:close()
    return 1
end

function test_close()
    local db = open_db()
    db:close()
    return 1
end

function test_multiple_inserts()
    local db = open_db()
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
    local db = open_db()
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
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER)")
    db:exec("INSERT INTO t VALUES (1)")
    -- do not close explicitly; finalizer should handle it
    return 1
end

-- ── Enhanced API tests: prepare / bind / step / reset / columns / helpers ──

function test_prepare_bind_step()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER, name TEXT, score REAL)")

    -- Insert via prepared statement: 3 placeholders, 3 values
    local stmt = db:prepare("INSERT INTO t VALUES (?, ?, ?)")
    stmt:bind(1, "Alice", 95.5)
    stmt:step()
    stmt:close()

    -- Read back
    local rows = db:exec("SELECT * FROM t")
    if #rows ~= 1 then return 0 end
    if rows[1].id ~= 1 then return 0 end
    if rows[1].name ~= "Alice" then return 0 end
    if rows[1].score < 95.4 or rows[1].score > 95.6 then return 0 end
    db:close()
    return 1
end

function test_prepare_select_where()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER, name TEXT)")
    db:exec("INSERT INTO t VALUES (1, 'Alice')")
    db:exec("INSERT INTO t VALUES (2, 'Bob')")
    db:exec("INSERT INTO t VALUES (3, 'Charlie')")

    -- 1 placeholder, 1 value
    local stmt = db:prepare("SELECT * FROM t WHERE id > ? ORDER BY id")
    stmt:bind(1)
    local rows = {}
    local row = stmt:step()
    while row do
        rows[#rows + 1] = row
        row = stmt:step()
    end
    stmt:close()

    if #rows ~= 2 then return 0 end
    if rows[1].name ~= "Bob" then return 0 end
    if rows[2].name ~= "Charlie" then return 0 end
    db:close()
    return 1
end

function test_prepare_nil_bind()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER, val TEXT)")

    -- 2 placeholders, 2 values (second is nil)
    local stmt = db:prepare("INSERT INTO t VALUES (?, ?)")
    stmt:bind(1, nil)
    stmt:step()
    stmt:close()

    local rows = db:exec("SELECT * FROM t")
    if #rows ~= 1 then return 0 end
    if rows[1].id ~= 1 then return 0 end
    if rows[1].val ~= nil then return 0 end
    db:close()
    return 1
end

function test_prepare_reset()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER, name TEXT)")

    -- Insert two rows with the same prepared statement
    local stmt = db:prepare("INSERT INTO t VALUES (?, ?)")
    stmt:bind(1, "Alice")
    stmt:step()
    stmt:reset()
    stmt:bind(2, "Bob")
    stmt:step()
    stmt:close()

    local rows = db:exec("SELECT * FROM t ORDER BY id")
    if #rows ~= 2 then return 0 end
    if rows[1].name ~= "Alice" then return 0 end
    if rows[2].name ~= "Bob" then return 0 end
    db:close()
    return 1
end

function test_columns()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER, name TEXT, score REAL)")

    local stmt = db:prepare("SELECT id, name, score FROM t")
    local cols = stmt:columns()
    if #cols ~= 3 then return 0 end
    if cols[1] ~= "id" then return 0 end
    if cols[2] ~= "name" then return 0 end
    if cols[3] ~= "score" then return 0 end
    stmt:close()
    db:close()
    return 1
end

function test_last_insert_rowid()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER PRIMARY KEY, name TEXT)")

    db:exec("INSERT INTO t (name) VALUES ('Alice')")
    local id = db:last_insert_rowid()
    if id ~= 1 then return 0 end

    db:exec("INSERT INTO t (name) VALUES ('Bob')")
    id = db:last_insert_rowid()
    if id ~= 2 then return 0 end
    db:close()
    return 1
end

function test_changes()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER, name TEXT)")

    db:exec("INSERT INTO t VALUES (1, 'Alice')")
    if db:changes() ~= 1 then return 0 end

    db:exec("INSERT INTO t VALUES (2, 'Bob')")
    db:exec("INSERT INTO t VALUES (3, 'Charlie')")
    if db:changes() ~= 1 then return 0 end

    db:exec("DELETE FROM t WHERE id > 1")
    if db:changes() ~= 2 then return 0 end
    db:close()
    return 1
end

function test_transaction()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER, name TEXT)")

    db:exec("BEGIN")
    db:exec("INSERT INTO t VALUES (1, 'Alice')")
    db:exec("INSERT INTO t VALUES (2, 'Bob')")
    db:exec("COMMIT")

    local rows = db:exec("SELECT * FROM t")
    if #rows ~= 2 then return 0 end

    -- Test rollback
    db:exec("BEGIN")
    db:exec("INSERT INTO t VALUES (3, 'Charlie')")
    db:exec("ROLLBACK")

    rows = db:exec("SELECT * FROM t")
    if #rows ~= 2 then return 0 end
    db:close()
    return 1
end

function test_prepare_error()
    -- Test that valid prepare + bind + step works for single-param queries
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER)")
    local stmt = db:prepare("INSERT INTO t VALUES (?)")
    stmt:bind(42)  -- 1 placeholder, 1 value
    stmt:step()
    stmt:close()
    local rows = db:exec("SELECT * FROM t")
    if #rows ~= 1 then return 0 end
    if rows[1].id ~= 42 then return 0 end
    db:close()
    return 1
end

function test_prepare_multi_rows()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER, val TEXT)")

    local stmt = db:prepare("INSERT INTO t VALUES (?, ?)")
    for i = 1, 100 do
        stmt:bind(i, "val_" .. i)
        stmt:step()
        stmt:reset()
    end
    stmt:close()

    local rows = db:exec("SELECT * FROM t")
    if #rows ~= 100 then return 0 end
    if rows[50].id ~= 50 then return 0 end
    if rows[100].val ~= "val_100" then return 0 end
    db:close()
    return 1
end

function test_blob()
    local db = open_db()
    db:exec("CREATE TABLE t (b BLOB)")
    db:exec("INSERT INTO t VALUES (X'00ff4142')")
    local rows = db:exec("SELECT b FROM t")
    if type(rows) ~= "table" or #rows ~= 1 then return 0 end
    if rows[1].b ~= string.char(0, 255, 65, 66) then return 0 end
    db:close()
    db:close()
    return 1
end

function test_bind_embedded_nul()
    local db = open_db()
    db:exec("CREATE TABLE t (val TEXT)")
    local s = "ab" .. string.char(0) .. "cd"
    local stmt = db:prepare("INSERT INTO t VALUES (?)")
    stmt:bind(s)
    stmt:step()
    stmt:close()
    local rows = db:exec("SELECT val FROM t")
    if type(rows) ~= "table" or #rows ~= 1 then return 0 end
    if rows[1].val ~= s then return 0 end
    if #rows[1].val ~= 5 then return 0 end
    db:close()
    return 1
end

function test_delete_state_without_close()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER)")
    db:exec("INSERT INTO t VALUES (1)")
    local stmt = db:prepare("SELECT id FROM t")
    -- intentionally leave db/stmt open; FakeluaDeleteState must not leak/crash
    return 1
end

function test_stmt_close_after_db_close()
    local db = open_db()
    db:exec("CREATE TABLE t (id INTEGER)")
    local stmt = db:prepare("SELECT id FROM t")
    db:close()
    stmt:close()
    return 1
end
