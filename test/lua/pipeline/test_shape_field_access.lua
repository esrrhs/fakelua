-- 字段访问: a.b 从 shape 中读字段类型
function test()
    local a = {x = 10, y = 20}
    local x = a.x
    local y = a.y
    return x + y
end
