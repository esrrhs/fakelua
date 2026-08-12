-- Test table error paths: bad argument types must throw
-- Covers: table.insert / table.remove / table.sort / table.concat / table.move

function test_table_insert_bad_arg()
    table.insert(true)
end

function test_table_remove_bad_arg()
    table.remove(true)
end

function test_table_sort_bad_arg()
    table.sort(true)
end

function test_table_concat_bad_arg()
    -- 第一个参数必须是 table
    table.concat(true)
end

function test_table_concat_bad_sep()
    -- sep 必须是 string
    table.concat({}, true)
end

function test_table_move_bad_arg()
    table.move(true, 1, 2, 3)
end
