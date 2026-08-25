-- JIT 以前把非 INT 下标默认成 1/0/#t：table.insert(t, 2.0, x) 插到位置 1，
-- table.remove(t, 2.0) 删最后一个，table.create(3.0, v) 得到空表。

function test_table_float_index()
    local t = {10, 20, 30}
    table.insert(t, 2.0, 99)
    if t[1] ~= 10 or t[2] ~= 99 or t[3] ~= 20 or t[4] ~= 30 then return 1 end

    local r = table.remove({1, 2, 3}, 2.0)
    if r ~= 2 then return 2 end

    local u = table.create(3.0, "x")
    if #u ~= 3 or u[1] ~= "x" or u[3] ~= "x" then return 3 end

    local a = {1, 2, 3, 4}
    local b = {}
    table.move(a, 2.0, 3.0, 1.0, b)
    if b[1] ~= 2 or b[2] ~= 3 or b[3] ~= nil then return 4 end

    local x, y = table.unpack({10, 20, 30}, 2.0, 3.0)
    if x ~= 20 or y ~= 30 then return 5 end

    -- 整数字面量快路径仍要正确
    local t2 = {10, 20, 30}
    table.insert(t2, 2, 99)
    if t2[2] ~= 99 then return 6 end

    return 5000
end
