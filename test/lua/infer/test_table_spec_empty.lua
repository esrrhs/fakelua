-- 空 Table 无法特化测试
function test_empty()
    local t = {}
    t.x = 10
    return t.x
end
