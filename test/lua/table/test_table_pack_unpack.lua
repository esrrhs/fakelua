function test_table_pack_unpack()
    local p = table.pack(10, 20, 30)
    local n = p.n
    local a, b, c = table.unpack(p)

    local t2 = {}
    table.move(p, 1, 3, 1, t2)
    local x, y, z = table.unpack(t2, 1, 3)

    return n + a + b + c + x + y + z
end
