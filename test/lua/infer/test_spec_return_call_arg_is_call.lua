-- f(n) = n + 1 is a math-param function.
-- caller returns f(f(n)): the argument to the outer f is itself a function call.
-- The snapshot-regenerating fixpoint re-runs RunTrialInference with
-- ResolveCallReturnType injecting f's T_INT return type into the call nodes,
-- so the inner f(n) node in the snapshot is T_INT, which is then passed as a
-- T_INT argument to the outer f — yielding a native int64_t return for caller.
-- caller(5) == 7, caller(3) == 5.
function f(n)
    return n + 1
end

function caller(n)
    return f(f(n))
end
