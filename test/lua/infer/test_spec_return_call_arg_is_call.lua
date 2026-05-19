-- f(n) = n + 1 is a math-param function.
-- caller returns f(f(n)): the argument to the outer f is itself a function call.
-- The return-type inferencer must recurse into the inner call's result type (T_INT)
-- and use it as the argument type for the outer call, yielding a native int64_t
-- return type for caller.
-- caller(5) == 7, caller(3) == 5.
function f(n)
    return n + 1
end

function caller(n)
    return f(f(n))
end
