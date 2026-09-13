-- 未声明属性访问退化测试
function test_undefined_field()
    local t = { x = 10 }
    t.y = 20
    return t.x + t.y
end
