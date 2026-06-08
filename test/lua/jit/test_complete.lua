local MY_CONST_2 = 100
local upvalue_int = 10
local upvalue_float = 20.0

local function foo(v)
    return v
end
local function my_abs(n)
    return n + 0
end
local function helper_bitwise_cvar(x)
    return foo(x & 1)
end
local function test_bitwise_cvar(y)
    local temp = y + 0.0
    return helper_bitwise_cvar(y)
end

local function helper_complete_spec(x, xf)
    local dummy = x + 1
    local dummy2 = xf + 1.0

    if ((x > 0)) then end
    if not (x > 0) then end

    x = foo(10)
    xf = foo(20.0)

    local v_mul = x * x
    local v_div = x / x
    local v_pow = x ^ x
    local v_fdiv_int = x // x
    local v_fdiv_float = xf // xf
    local v_mod_int = x % x
    local v_mod_float = xf % xf

    local val_const = 0
    val_const = MY_CONST_2

    local x_local = 5
    if x_local > my_abs(foo(x)) then end

    local y = x + 1
    local res = 0
    do
        local y = x + 2
        res = y
    end

    local sum = 0
    for i = foo(1), foo(5) do
        sum = sum + i
    end

    local t = { [x] = 2 }

    local le_val = xf <= foo(xf)

    local not_val = ~foo(x)

    return val_const + res + sum + t[x] + (le_val and 1 or 0) + not_val + dummy - dummy + dummy2 - dummy2 + v_mul - v_mul + v_div - v_div + v_pow - v_pow + v_fdiv_int - v_fdiv_int + v_fdiv_float - v_fdiv_float + v_mod_int - v_mod_int + v_mod_float - v_mod_float
end

function test_complete(x, xf)
    local temp = x + 0
    local tempf = xf + 0.0
    return helper_complete_spec(x, xf) + test_bitwise_cvar(xf)
end
