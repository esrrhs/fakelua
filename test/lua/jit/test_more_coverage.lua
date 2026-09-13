local MY_CONST = 42

local function my_abs(n)
    return n + 0
end

function test_more_coverage(x, xf)
    -- Table variable access (VarKind != kSimple)
    local t = { a = 1 }
    local val1 = t.a + x

    -- Global constant of type T_INT
    local val2 = x + MY_CONST

    -- Unary bitnot on specialized int
    local val3 = 0
    val3 = ~x

    -- Unary bitnot on specialized float (degraded)
    local val4 = 0.0
    val4 = ~xf

    -- Unary not
    local val5 = not t

    -- Calling specialized function with dynamic argument
    local glob = 10
    local val7 = my_abs(glob)

    -- TryCompileNativeBoolExpr branches
    if x then end
    if -x then end
    if not x then end
    if 1 then end
    if x > 0 and xf then end
    if x + 1 then end

    return val1 + val2 + val3 + val4
end

function test_shadow_coverage(x)
    local res = 0
    do
        local x = 6
        res = x
    end
    return res
end

function test_table_spec(x)
    local t = { [x] = 2 }
    return t[x]
end

function test_for_no_step(x)
    local sum = 0
    for i = 1, x do
        sum = sum + i
    end
    return sum
end
