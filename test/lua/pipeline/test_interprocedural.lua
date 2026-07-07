-- 跨函数推导: 返回值类型传递
function make()
    return {x = 1, y = 2}
end

function test()
    local p = make()
    return p.x + p.y
end
