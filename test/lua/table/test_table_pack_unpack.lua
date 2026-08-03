function test_table_pack_unpack()
    local p = table.pack(10, 20, 30)
    local n = p.n
    local a, b, c = table.unpack(p)

    local t2 = {}
    table.move(p, 1, 3, 1, t2)
    local x, y, z = table.unpack(t2, 1, 3)

    -- 当 i > j 时返回空元组
    local empty_t = {}
    local empty_a, empty_b = table.unpack(empty_t, 2, 1)
    if empty_a ~= nil or empty_b ~= nil then return 0 end

    return n + a + b + c + x + y + z
end
