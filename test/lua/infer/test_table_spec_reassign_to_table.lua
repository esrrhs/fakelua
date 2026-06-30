-- 变量先赋整数值，再赋值为table构造器，验证table仍可特化
function test_reassign_to_spec_table()
    local a = 1
    a = { x = 10, y = 20 }
    return a.x + a.y
end

-- 变量先赋nil，再赋值为table
function test_nil_to_table()
    local t = nil
    t = { [1] = 100, [2] = 200 }
    return t[1] + t[2]
end
