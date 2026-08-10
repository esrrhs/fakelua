-- Test table.sort with default comparator (ascending sort)
-- Also test table.move overlap and no-overlap scenarios

function test_table_sort_custom_cmp()
    local t = {5, 3, 8, 1, 4, 2, 7, 6}
    -- Sort ascending using default comparator
    table.sort(t)
    if t[1] ~= 1 then return 1 end
    if t[2] ~= 2 then return 2 end
    if t[8] ~= 8 then return 3 end

    -- Sort strings in ascending (default) order
    local t2 = {"banana", "apple", "cherry", "date"}
    table.sort(t2)
    if t2[1] ~= "apple" then return 4 end
    if t2[4] ~= "date" then return 5 end
    return 5000
end

function test_table_move_overlap()
    -- table.move with overlapping regions on the same table
    local t = {10, 20, 30, 40, 50}
    -- Move elements [1..3] to start at [2] on same table (overlap, backward copy)
    table.move(t, 1, 3, 2)
    if t[2] ~= 10 then return 1 end
    if t[3] ~= 20 then return 2 end
    if t[4] ~= 30 then return 3 end
    return 5000
end

function test_table_move_no_overlap()
    -- table.move with non-overlapping regions
    local src = {1, 2, 3, 4, 5}
    local dst = {10, 20, 30, 40, 50}
    table.move(src, 1, 3, 3, dst)
    if dst[3] ~= 1 then return 1 end
    if dst[4] ~= 2 then return 2 end
    if dst[5] ~= 3 then return 3 end
    return 5000
end

function test_table_concat_float()
    local t = {1.5, 2.5, 3.5}
    local s = table.concat(t, ",")
    if s == nil then return 1 end
    if string.len(s) < 5 then return 2 end
    return 5000
end

function test_table_sort_single()
    -- table.sort with single element (len <= 1, should be no-op)
    local t = {42}
    table.sort(t)
    if t[1] ~= 42 then return 1 end

    -- Empty table
    local t2 = {}
    table.sort(t2)
    return 5000
end
