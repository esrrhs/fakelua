-- MergeType(T_INT, T_FLOAT) = T_DYNAMIC.
-- x is first assigned the integer literal 1 (T_INT), then the float literal
-- 1.5 (T_FLOAT).  TypeEnvironment::Update calls MergeType(T_INT, T_FLOAT),
-- which falls through to the "different non-unknown types" path and returns
-- T_DYNAMIC.  The post-pass then updates the LocalVar initialiser's EvalType
-- to T_DYNAMIC so CGen emits a CVar declaration.
-- Expected return value: 1.5.
function test()
    local x = 1
    x = 1.5
    return x
end
