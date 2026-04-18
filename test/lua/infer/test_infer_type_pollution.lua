-- Type pollution: a is initially T_INT, then mutated to a string (T_DYNAMIC).
-- b = a + 5 is computed when a is still 10, so its runtime value must be 15.
-- The compiler must handle the case where a variable used in a numeric expression
-- ends up declared as CVar because it is later mutated.
function test()
    local a = 10         -- initially T_INT, later mutated → CVar
    local b = a + 5      -- depends on a; a is CVar at this point (value 10)
    a = "hello"          -- a mutated to string (T_DYNAMIC)
    local c = a          -- c = "hello" (T_DYNAMIC)
    return b             -- runtime value: 15
end
