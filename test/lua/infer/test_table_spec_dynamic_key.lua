-- 动态 Key 访问退化测试
function test_dynamic_key()
    local t = { x = 10, y = 20 }
    local k = "x"
    return t[k]
end
