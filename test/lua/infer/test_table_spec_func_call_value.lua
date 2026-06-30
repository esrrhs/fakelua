-- 隐式元素是函数调用（多返回），无法特化整个table
function test_func_call_element()
    local t = { get_val() }
    return t[1]
end
function get_val() return 42 end
