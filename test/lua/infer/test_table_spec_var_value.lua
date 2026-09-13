-- 变量值作为table字段，可特化
function test_var_value()
    local x = 10
    local y = 20
    local t = { a = x, b = y }
    return t.a + t.b
end

-- 变量值写入后再读取
function test_var_value_write()
    local x = 10
    local t = { a = x }
    local y = 99
    t.a = y
    return t.a
end
