function test_table_sort_bad_cmp()
    local t1 = { 5, 2, 8, 1, 9 }
    table.sort(t1, 123)
end
