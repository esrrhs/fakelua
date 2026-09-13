-- func(n) = n * 2 is a math-param function.
-- outer declares a local variable INSIDE an if block (not at the top level):
--   when n > 0: local x = func(n); return x + 1
--   when n <= 0: return 0
-- The snapshot-regenerating fixpoint re-runs RunTrialInference with
-- ResolveCallReturnType injecting func's T_INT return type, so the call node
-- for func(n) and the local x are T_INT in the snapshot.  ComputeReturnTypeFromSnapshot
-- then sees x+1 as T_INT, giving outer a native int64_t return type.
-- outer(5) == 11 (5*2+1), outer(-1) == 0.
function func(n)
    return n * 2
end

function outer(n)
    if n > 0 then
        local x = func(n)
        return x + 1
    end
    return 0
end
