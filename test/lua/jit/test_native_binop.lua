local function foo(x)
    return x
end

local function test_native_binop_int(x, y)
    local dummy1 = x + 1
    local dummy2 = y + 1

    local v_add = foo(x + y)
    local v_sub = foo(x - y)
    local v_mul = foo(x * y)
    local v_div = foo(x / y)
    local v_pow = foo(x ^ y)
    local v_fdiv = foo(x // y)
    local v_mod = foo(x % y)
    local v_and = foo(x & y)
    local v_or = foo(x | y)
    local v_xor = foo(x ~ y)
    local v_shl = foo(x << y)
    local v_shr = foo(x >> y)
    return 1
end

local function test_native_binop_float(x, y)
    local dummy1 = x + 1.0
    local dummy2 = y + 1.0

    local v_add = foo(x + y)
    local v_sub = foo(x - y)
    local v_mul = foo(x * y)
    local v_div = foo(x / y)
    local v_pow = foo(x ^ y)
    local v_fdiv = foo(x // y)
    local v_mod = foo(x % y)
    return 1
end

function test_entry(x, y, xf, yf)
    local temp1 = x + 0
    local temp2 = y + 0
    local temp3 = xf + 0.0
    local temp4 = yf + 0.0
    return test_native_binop_int(x, y) + test_native_binop_float(xf, yf)
end
