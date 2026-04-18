-- Return-stale-T_INT bug: the return statement appears in source BEFORE the
-- assignment that degrades x to T_DYNAMIC, so single-pass type inference stamps
-- EvalType = T_INT on the return expression.  In subsequent loop iterations
-- where x was already mutated to a string, the stale EvalType caused CGen to
-- emit  return (CVar){VAR_INT, x.data_.i}  -- returning a garbage integer CVar
-- instead of the string CVar.  With the fix, CompileExp is always used for
-- return values, so the CVar is returned directly.
-- Expected return value: "modified"  (x becomes "modified" at i=2, return fires at i=4).
function test()
    local x = 1
    for i = 1, 5 do
        if i >= 4 then
            return x          -- return before mutation in source; stale EvalType = T_INT
        end
        if i >= 2 then
            x = "modified"    -- degrades x to T_DYNAMIC; appears AFTER the return check
        end
    end
    return x
end
