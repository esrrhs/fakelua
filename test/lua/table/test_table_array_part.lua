local function checksum(t)
    local s = 0
    local n = 0
    for k, v in pairs(t) do
        if type(k) == "number" then
            s = s + math.floor(k) * 31 + math.floor(v)
        else
            s = s + 7
        end
        n = n + 1
    end
    return math.floor(s * 1000 + n)
end

function test_table_array_part()
    local total = 0

    -- 顺序追加与 # / t[#t+1]
    local a = {}
    for i = 1, 100 do a[#a + 1] = i * 2 end
    total = total + checksum(a) + #a

    -- 逆序填充：先进哈希，最后一个键到位时整体迁入数组
    local b = {}
    for i = 50, 1, -1 do b[i] = i end
    total = total + checksum(b) + #b

    -- 从 2 开始填充（Lua 风格数组部分重哈希）
    local c = {}
    for i = 2, 200 do c[i] = i end
    total = total + checksum(c)
    c[1] = 1
    total = total + checksum(c) + #c

    -- 尾部置 nil 后再追加
    local d = {}
    for i = 1, 20 do d[i] = i end
    d[20] = nil
    d[19] = nil
    total = total + checksum(d) + #d
    d[#d + 1] = 500
    total = total + checksum(d) + #d

    -- 中间置 nil：遍历跳过空洞，# 取连续前缀
    local e = {}
    for i = 1, 10 do e[i] = i end
    e[5] = nil
    total = total + checksum(e)
    e[5] = 55
    total = total + checksum(e) + #e

    -- pairs 中清空整张表
    local f = {}
    for i = 1, 30 do f[i] = i end
    f.x = 1
    for k, _ in pairs(f) do f[k] = nil end
    total = total + checksum(f) + #f

    -- 混合键、0 与负数键、稀疏键
    local g = { 10, 20, 30, x = 1 }
    g[0] = 5
    g[-1] = 6
    g[1000000] = 7
    g[4] = 40
    g[5] = 50
    g[2.0] = 21
    total = total + checksum(g) + #g

    -- table.insert / table.remove
    local h = {}
    for i = 1, 40 do table.insert(h, i) end
    table.insert(h, 1, 99)
    table.remove(h)
    table.remove(h, 1)
    total = total + checksum(h) + #h

    -- 稀疏后补齐
    local m = {}
    m[3] = 3
    m[5] = 5
    m[1] = 1
    m[2] = 2
    m[4] = 4
    total = total + checksum(m) + #m

    -- table.move 数组段自移动与跨表移动
    local mv_src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }
    table.move(mv_src, 1, 5, 6, mv_src)
    total = total + checksum(mv_src) + #mv_src
    local mv_dst = {}
    table.move(mv_src, 1, 10, 1, mv_dst)
    total = total + checksum(mv_dst) + #mv_dst
    table.move(mv_src, 3, 7, 1, mv_src)
    total = total + checksum(mv_src) + #mv_src

    -- 浮点等价键与非整数浮点键
    local flt = {}
    flt[1.0] = 100
    flt[2.0] = 200
    flt[1.5] = 150
    flt[2.5] = 250
    total = total + math.floor(flt[1]) + math.floor(flt[2]) + math.floor(flt[1.5]) + math.floor(flt[2.5]) + #flt
    total = total + checksum(flt) + #flt
    flt[1] = 111
    total = total + math.floor(flt[1.0])

    -- 大规模连续扩容与截断收缩
    local big = {}
    for i = 1, 512 do big[i] = i end
    total = total + #big + big[1] + big[512]
    for i = 512, 257, -1 do big[i] = nil end
    total = total + #big
    for i = 257, 512 do big[i] = i * 2 end
    total = total + #big + checksum(big)

    -- 阶梯跳跃打洞再逆向补齐
    local sk = {}
    sk[1] = 10
    sk[4] = 40
    sk[8] = 80
    sk[16] = 160
    total = total + #sk
    sk[2] = 20
    sk[3] = 30
    total = total + #sk
    for i = 5, 7 do sk[i] = i * 10 end
    total = total + #sk
    for i = 9, 15 do sk[i] = i * 10 end
    total = total + #sk + checksum(sk)

    -- table.create 预分配数组配合数组操作
    local tc = table.create(30, 7)
    total = total + #tc + tc[1] + tc[30]
    tc[15] = 99
    table.insert(tc, 100)
    total = total + #tc + tc[31] + checksum(tc)

    -- table.pack / table.unpack
    local pk = table.pack(11, 22, 33, 44, 55)
    local u1, u2, u3, u4, u5 = table.unpack(pk, 1, 5)
    total = total + u1 + u2 + u3 + u4 + u5 + pk.n + checksum(pk)

    -- ipairs 遍历数组段与遇到空洞截断
    local ip_tbl = { 10, 20, 30, 40, 50 }
    ip_tbl[3] = nil
    local ip_sum = 0
    for idx, val in ipairs(ip_tbl) do
        ip_sum = ip_sum + idx * 100 + val
    end
    total = total + ip_sum

    -- 边界极值键与负数键
    local ext = { 1, 2, 3 }
    ext[0] = 99
    ext[-10] = 88
    ext[2147483647] = 77
    total = total + #ext + ext[1] + ext[2] + ext[3] + ext[0] + ext[-10] + ext[2147483647]

    return math.tointeger(total) or math.floor(total)
end
