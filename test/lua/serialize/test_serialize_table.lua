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
