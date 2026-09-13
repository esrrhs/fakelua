local function foo(v)
    return v
end

local function helper_int(x)
    local dummy = x + 1
    -- Exercise all arithmetic ops inside specialized int context
    local v_mul = foo(x * x)
    local v_div = foo(x / x)
    local v_pow = foo(x ^ x)
    local v_fdiv = foo(x // x)
    local v_mod = foo(x % x)
    return 1
end

local function helper_float(xf)
    local dummy = xf + 1.0
    -- Exercise all arithmetic ops inside specialized float context
    local v_mul = foo(xf * xf)
    local v_div = foo(xf / xf)
    local v_pow = foo(xf ^ xf)
    local v_fdiv = foo(xf // xf)
    local v_mod = foo(xf % xf)
    return 1
end

function test(x, xf)
    local temp = x + 0
    local tempf = xf + 0.0
    return helper_int(x) + helper_float(xf)
end
