-- Integer arguments must use the same checked native implementation on every
-- backend; these cases guard against reintroducing stale JIT-only copies.

function test_table_move_over_cap()
    local src = {}
    src[1] = 11
    src[10000001] = 22
    local dst = {}
    dst[1] = 1
    dst[2] = 2
    dst[10000001] = 3
    local ret = table.move(src, 1, 10000001, 1, dst)
    if ret ~= dst then return 1 end
    if dst[1] ~= 11 then return 2 end
    if dst[2] ~= nil then return 3 end
    if dst[10000001] ~= 22 then return 4 end
    return 5000
end

function test_table_move_src_wrap()
    table.move({}, math.mininteger, math.maxinteger, 1, {})
    return 1
end

function test_table_move_src_overflow()
    -- 0 .. maxinteger 的个数是 2^63，装不进 int64
    table.move({}, 0, math.maxinteger, 1, {})
    return 1
end

function test_table_move_dst_wrap()
    -- 源区间只有 2 个元素，终点越过 maxinteger
    table.move({ 1, 2 }, 1, 2, math.maxinteger, {})
    return 1
end

function test_table_unpack_over(n)
    table.unpack({}, 1, n)
    return 1
end

function test_table_unpack_wrap()
    table.unpack({}, math.mininteger, math.maxinteger)
    return 1
end

function test_table_unpack_ok()
    local t = {}
    t[1] = 4
    t[3] = 6
    local a, b, c = table.unpack(t, 1, 3)
    if a ~= 4 or b ~= nil or c ~= 6 then return 0 end
    local x = table.unpack(t, math.maxinteger, math.maxinteger)
    if x ~= nil then return 0 end
    t[math.maxinteger] = 8
    x = table.unpack(t, math.maxinteger, math.maxinteger)
    if x ~= 8 then return 0 end
    return 5000
end

-- 目标空表的预扩容装不进一块引擎内存时，必须退化为逐元素插入且结果正确。
function test_table_move_presize_skip()
    local src = {}
    src[1] = 7
    src[100000] = 9
    local dst = {}
    table.move(src, 1, 100000, 1, dst)
    if dst[1] ~= 7 then return 1 end
    if dst[2] ~= nil then return 2 end
    if dst[100000] ~= 9 then return 3 end
    return 5000
end

-- 同一张表内重叠移动（t > f 且 t <= e）走反向拷贝。
function test_table_move_overlap_backward()
    local t = { 1, 2, 3, 4, 5 }
    table.move(t, 1, 4, 2)
    if t[1] ~= 1 or t[2] ~= 1 or t[3] ~= 2 or t[4] ~= 3 or t[5] ~= 4 then return 1 end
    return 5000
end

-- e < f 时是空移动，直接返回目标表。
function test_table_move_empty_range()
    local dst = { 1 }
    local r = table.move({ 1, 2 }, 2, 1, 1, dst)
    if r ~= dst or dst[1] ~= 1 then return 1 end
    return 5000
end
