local upvalue_int = 10
local upvalue_float = 20.0

local function foo(v)
    return v
end
local function my_abs(n)
    return n + 0
end
local function my_mixed_func(n, name)
    return n + 1
end
local function helper_spec_mixed(x)
    return x + my_mixed_func(x, "hello")
end
local function helper_spec_str(x)
    return x + my_abs "5"
end
local function non_math_helper(x)
    return x
end
local function helper_spec_non_math(x)
    return x + non_math_helper(5)
end
local function my_abs_dynamic(n)
    if n > 0 then
        return n
    else
        return "not a number"
    end
end
local function helper_spec_dynamic_ret(x)
    return x + my_abs_dynamic(5)
end
local function helper_spec_dynamic_arg(x)
    return x + my_abs(foo(x))
end
local function helper_upvalue_spec(x)
    local dummy = x + 1.0
    return dummy + upvalue_int + upvalue_float
end
local function helper_bool_ret(x)
    return x > 0
end
local function test_bool_ret_spec(x)
    local dummy = x + 1
    return helper_bool_ret(x)
end
local function helper_user_func(x)
    return x + 1
end
local function test_user_func_spec(x)
    local dummy = x + 1
    return x + helper_user_func(x)
end
local function helper_user_func_float(x)
    return x + 1.0
end
local function test_user_func_spec_float(x)
    local dummy = x + 1.0
    return x + helper_user_func_float(x)
end
local function test_set_table_fallback(t, k, v)
    FAKELUA_SET_TABLE(t, k, v)
end
local function helper_default_float(x)
    local dummy = x + 1.0
    if x > 0 then
        return x + 1.0
    else
        error("error")
    end
end
local function helper_default_int(x)
    local dummy = x + 1
    if x > 0 then
        return x + 1
    end
end

local function helper_specs_all(x)
    local dummy = x + 1

    local val1 = helper_spec_mixed(x)
    local val2 = 0
    if false then
        val2 = helper_spec_str(x)
    end
    local val3 = helper_spec_non_math(x)
    local val4 = helper_spec_dynamic_ret(x)
    local val5 = helper_spec_dynamic_arg(x)
    local val6 = helper_upvalue_spec(x)
    local val7 = 0
    if test_bool_ret_spec(x) then
        val7 = 1
    end
    local val8 = test_user_func_spec(x)
    local val9 = test_user_func_spec_float(x)

    local val10 = helper_default_float(x) or 0.0
    local val11 = helper_default_int(x) or 0

    local val12 = -x
    local val13 = 0
    if false then
        val13 = #val1
    end

    while foo(1) do
        break
    end

    if false then
        local b = x > 0
        local res_dyn = my_abs(b & 1)
    end

    local tbl = {}
    test_set_table_fallback(tbl, "mykey", 123)

    return val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8 + val9 + val10 + val11 + val12 - val12 + val13 - val13 + dummy - dummy
end

function test_specs_helper(y)
    local temp = y + 0
    return helper_specs_all(y)
end
