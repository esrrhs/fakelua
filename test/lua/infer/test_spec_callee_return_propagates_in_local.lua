-- Three-level chain: inner(n) returns n*2 (T_INT), middle(n) stores the result
-- in a local variable declared inside an if block, adds 1, and returns it.
-- Without snapshot regeneration (old BuildLocalVarExtensions/EvalReturnExpType path),
-- the local variable `r` would remain T_DYNAMIC in the snapshot because the
-- function-call node for inner(n) was T_DYNAMIC in the initial snapshot.  With
-- the new snapshot-regenerating fixpoint (ResolveCallReturnType), inner's return
-- the RunTrialInference for middle, so `r` is T_INT in the snapshot and the
-- return expression `r + 1` evaluates to T_INT directly.
-- outer(n) calls middle(n) and returns the result unchanged.

function inner(n)
    return n * 2
end

function middle(n)
    if n > 0 then
        local r = inner(n)
        return r + 1
    end
    return 0
end

function outer(n)
    return middle(n)
end
