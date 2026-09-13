-- func(n) = n * 2 is a math-param function.
-- caller returns func(n) + func(n): both sides of the binop are function-call
-- results, not local variables.  The snapshot-regenerating fixpoint re-runs
-- RunTrialInference with ResolveCallReturnType injecting func's T_INT return type,
-- so both func(n) call nodes in the snapshot are T_INT, and T_INT+T_INT=T_INT
-- gives caller a native int64_t return type.
-- caller(5) == 20, caller(3) == 12.
function func(n)
    return n * 2
end

function caller(n)
    return func(n) + func(n)
end
