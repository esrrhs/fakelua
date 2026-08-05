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

    -- 当第 5 个参数显式为 nil 时回退为 a1 (table.move(a1, f, e, t, nil))
    local move_nil = {1, 2, 3}
    table.move(move_nil, 1, 2, 2, nil)
    if move_nil[1] ~= 1 or move_nil[2] ~= 1 or move_nil[3] ~= 2 then return 0 end

    return n + a + b + c + x + y + z
end
