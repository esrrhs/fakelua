-- Assign-to-CVar-via-stale-T_INT bug: sum is initialised as T_INT but later
-- degraded to T_DYNAMIC by  sum = "done"  in the else branch.  The type
-- inferencer (single-pass) marks the  sum = sum + i  assign node with
-- EvalType = T_INT *before* seeing the else branch.  Without the fix CGen
-- would emit  sum = (int64_t expression)  -- invalid C because sum is CVar.
-- With the fix the typed_native_vars_ check is used instead of EvalType, so
-- CVar arithmetic is emitted and the program compiles and runs correctly.
-- Expected return value: "done"  (sum becomes "done" when i reaches 4 or 5).
function test()
    local sum = 0
    for i = 1, 5 do
        if i <= 3 then
            sum = sum + i   -- sum is CVar after post-pass; must use CVar arithmetic
        else
            sum = "done"    -- degrades sum to T_DYNAMIC
        end
    end
    return sum
end
