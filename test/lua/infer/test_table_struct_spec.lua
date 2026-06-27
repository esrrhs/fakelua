-- table struct specialization 测试
function test_struct_spec()
    local t = { x = 10, y = 20 }
    t.x = 99
    return t.x + t.y
end
