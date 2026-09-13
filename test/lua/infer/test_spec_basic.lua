-- 基本特化：两字段 table，写后读
function test_basic()
    local t = { x = 10, y = 20 }
    t.x = 99
    return t.x + t.y
end
