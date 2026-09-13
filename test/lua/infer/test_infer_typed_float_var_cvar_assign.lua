-- Typed float native var assigned from a dynamic function call: exercises
-- c_gen.cpp lines 1735-1741 (CVar extraction fallback for typed native vars).
-- local x = 1.0 is declared as double (T_FLOAT), then x = helper() assigns
-- a T_DYNAMIC CVar result. TryCompileNativeExpr fails for helper(), so the
-- code falls through to extract .data_.f from the CVar.
-- Result: helper() returns 3 as T_DYNAMIC, extracted as float -> 3.0.
function helper()
    return 3
end

function test()
    local x = 1.0
    x = helper()
    return x
end
