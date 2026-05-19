-- func(n) = n * 2 is a math-param function.
-- caller returns func(n) + func(n): both sides of the binop are function-call
-- results, not local variables.  The return-type inferencer must evaluate the
-- binop recursively (via EvalReturnExpType) and resolve both func(n) calls to
-- T_INT, giving caller a native int64_t return type.
-- caller(5) == 20, caller(3) == 12.
function func(n)
    return n * 2
end

function caller(n)
    return func(n) + func(n)
end
