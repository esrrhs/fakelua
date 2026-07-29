function test_table_sort()
    local t = {5, 2, 8, 1, 9}
    table.sort(t)
    if t[1] ~= 1 or t[2] ~= 2 or t[3] ~= 5 or t[4] ~= 8 or t[5] ~= 9 then
        return 0.0
    end

    local t2 = {10, 50, 20, 40, 30}
    table.sort(t2, function(a, b) return a > b end)
    if t2[1] ~= 50 or t2[2] ~= 40 or t2[3] ~= 30 or t2[4] ~= 20 or t2[5] ~= 10 then
        return 0.0
    end

    return 100.0
end
