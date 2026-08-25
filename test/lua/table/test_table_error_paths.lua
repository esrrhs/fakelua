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

function test_table_concat_too_many()
    table.concat({}, ",", 1, 20000000)
end

function test_table_concat_min_max_range()
    table.concat({}, ",", math.mininteger, math.maxinteger)
end

function test_table_create_too_many()
    table.create(math.maxinteger, 0)
end

function test_table_insert_2pow63()
    table.insert({1, 2, 3}, 2^63, 99)
end

function test_table_remove_2pow63()
    table.remove({1, 2, 3}, 2^63)
end

function test_table_sort_nan()
    -- NaN 破坏 C++ strict weak ordering，std::stable_sort 是 UB
    table.sort({1, 0.0 / 0.0, 2})
end
