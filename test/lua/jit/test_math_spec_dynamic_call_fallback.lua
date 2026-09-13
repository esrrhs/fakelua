local function foo()
    return 5
end

local function my_abs(n)
    return n + 0
end

function test(y)
    -- foo() is not native-compilable, so native_expr.empty() -> fallback to CompileExp
    local val = my_abs(foo()) + 1
    return val
end
