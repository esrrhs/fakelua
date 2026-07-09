-- 逃逸测试: return 的变量标记为逃逸
function test()
    local a = {x = 1}
    return a
end
