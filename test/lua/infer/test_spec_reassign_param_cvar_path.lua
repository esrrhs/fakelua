-- Bug 2 fix: spec_param_types_ is erased for a math param that is reassigned
-- through the CVar fallback path (cvar_helper is not specialised, returns CVar).
-- After the erasure, InferArgTypeForSpec falls through to GetNativeVarType which
-- still returns T_INT (n is declared as int64_t in the spec), so subsequent
-- arithmetic on n remains native and correct.
--
-- n is a math param (n + 1 in the return triggers specialisation).
-- `n = cvar_helper(n)` reassigns n via the CVar fallback path.
-- Bug 2 fix: spec_param_types_["n"] is erased; GetNativeVarType("n") = T_INT.
-- `return n + 1` still compiles as native int64_t addition.
--
-- test(5) => n = cvar_helper(5) = 7, return 7 + 1 = 8.
-- test(10) => n = cvar_helper(10) = 12, return 12 + 1 = 13.
-- test(0)  => n = cvar_helper(0)  = 2, return 2 + 1 = 3.

function cvar_helper(x)
    return x + 2
end

function test(n)
    n = cvar_helper(n)
    return n + 1
end
