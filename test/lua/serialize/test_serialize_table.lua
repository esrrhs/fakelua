package "SerializeTest"

-- 表往返：嵌套表、混合键类型、空表
function test_table()
    -- 空表
    local e = {}
    local bin = serialize.encode(e)
    if not bin then return 0 end
    local d = serialize.decode(bin)
    if type(d) ~= "table" then return 0 end
    if d ~= nil and d.x ~= nil then return 0 end

    -- 嵌套 + 混合类型
    local t = {
        id = 100,
        score = 3.5,
        ok = true,
        empty = nil,
        name = "nested",
        child = { a = 1, b = { c = 2 } }
    }
    bin = serialize.encode(t)
    if not bin then return 0 end
    d = serialize.decode(bin)
    if d.id ~= 100 then return 0 end
    if d.name ~= "nested" then return 0 end
    if d.ok ~= true then return 0 end
    if d.score < 3.5 - 1e-9 or d.score > 3.5 + 1e-9 then return 0 end
    if d.child == nil then return 0 end
    if d.child.a ~= 1 then return 0 end
    if d.child.b == nil then return 0 end
    if d.child.b.c ~= 2 then return 0 end
    return 1
end

function test_cycle_throw()
    local t = { a = 1 }
    t.self = t
    serialize.encode(t)
end

-- 超过 quick_data_ 容量的数组，往返后键 1..9 必须齐全且不重复
function test_array_9()
    local t = {1, 2, 3, 4, 5, 6, 7, 8, 9}
    local bin = serialize.encode(t)
    if not bin then return 0 end
    local d = serialize.decode(bin)
    if type(d) ~= "table" then return 0 end
    for i = 1, 9 do
        if d[i] ~= i then return 0 end
    end
    return 1
end

function test_decode_too_deep()
    -- TAG_TABLE + count=1 + key int 1 + nested value, 70 times over nil
    local payload = string.char(0)
    local i = 1
    while i <= 70 do
        payload = string.char(7, 1, 3, 2) .. payload
        i = i + 1
    end
    serialize.decode(payload)
end

function test_decode_huge_table()
    -- TAG_TABLE + huge varint count with no payload
    serialize.decode(string.char(7, 255, 255, 255, 255, 15))
end
