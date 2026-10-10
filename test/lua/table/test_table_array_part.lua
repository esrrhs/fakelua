local function checksum(t)
    local s = 0
    local n = 0
    for k, v in pairs(t) do
        if type(k) == "number" then
            s = s + k * 31 + v
        else
            s = s + 7
        end
        n = n + 1
    end
    return s * 1000 + n
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

    return total
end
