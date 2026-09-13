-- 普通函数参数类型退化测试
function get_x(tbl)
    return tbl.x
end

function test_func_param_degrade()
    local t = { x = 42 }
    return get_x(t)
end
