function test_table_insert_remove()
    local t = {}
    table.insert(t, 10)
    table.insert(t, 20)
    table.insert(t, 30)
    table.insert(t, 2, 15) -- [10, 15, 20, 30]

    local rem1 = table.remove(t, 3) -- removes 20 -> [10, 15, 30]
    local rem2 = table.remove(t)    -- removes 30 -> [10, 15]

    local len = #t
    return t[1] + t[2] + rem1 + rem2 + len
end
