-- 覆盖标准库表函数在超过 quick_data_（8 槽）之后的行为。
-- 此前原生实现在 8 槽用满后会把整数键转成十进制字符串按 StringId 存储，
-- 与 JIT 侧按 VAR_INT 键的哈希查找不兼容，导致第 9 个及之后的元素静默丢失。
-- 失败时返回出错的步骤号，全部通过返回 8000。

function test_table_large_seq()
    local N = 40

    -- 1. table.move 到空表：跨越 8 槽边界后元素必须仍可读回
    local src = {}
    for i = 1, N do src[i] = i * 3 end
    local dst = {}
    table.move(src, 1, N, 1, dst)
    if #dst ~= N then return 1 end
    for i = 1, N do
        if dst[i] ~= i * 3 then return 2 end
    end

    -- 2. table.insert 追加后逐个读回
    local t = {}
    for i = 1, N do table.insert(t, i) end
    if #t ~= N then return 3 end
    for i = 1, N do
        if t[i] ~= i then return 4 end
    end

    -- 3. table.insert 指定位置（触发整段后移）
    table.insert(t, 1, 999)
    if #t ~= N + 1 then return 5 end
    if t[1] ~= 999 then return 6 end
    for i = 1, N do
        if t[i + 1] ~= i then return 7 end
    end

    -- 4. table.remove 从头部移除，剩余元素整体前移
    local removed = table.remove(t, 1)
    if removed ~= 999 then return 8 end
    if #t ~= N then return 9 end
    for i = 1, N do
        if t[i] ~= i then return 10 end
    end

    -- 5. table.concat 必须看到全部元素
    local c = {}
    for i = 1, 12 do c[i] = i end
    if table.concat(c, ",") ~= "1,2,3,4,5,6,7,8,9,10,11,12" then return 11 end

    -- 6. table.sort 覆盖全部元素而不只是前 8 个
    local s = {}
    for i = 1, N do s[i] = N + 1 - i end
    table.sort(s)
    for i = 1, N do
        if s[i] ~= i then return 12 end
    end

    -- 7. table.unpack 跨边界
    local u = {}
    for i = 1, 12 do u[i] = i * 2 end
    local a, b, l = table.unpack(u, 1, 3)
    if a ~= 2 or b ~= 4 or l ~= 6 then return 13 end

    -- 8. table.pack 收集超过 8 个参数
    local p = table.pack(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12)
    if p.n ~= 12 then return 14 end
    for i = 1, 12 do
        if p[i] ~= i then return 15 end
    end

    -- 9. 原生写入的表随后由 JIT 侧继续追加，两侧键表示必须一致
    local mixed = {}
    table.move(src, 1, 20, 1, mixed)
    mixed[21] = 63
    if #mixed ~= 21 then return 16 end
    if mixed[21] ~= 63 then return 17 end
    if mixed[20] ~= 60 then return 18 end

    return 8000
end
