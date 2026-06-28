-- 测试特化字段直接解引用读取
function test_direct()
    local t = { x = 10, y = 20 }
    t.x = 100
    
    -- 直接读取属性（这里会生成直连解引用！）
    local sum = t.x + t.y
    return sum
end
