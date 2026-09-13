-- 混合数组和对象的 Table 无法特化测试
function test_mixed()
    local t = { 100, x = 200 }
    return t.x
end
