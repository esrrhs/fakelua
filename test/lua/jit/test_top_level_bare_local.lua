-- Top-level bare local variable with no initializer.
-- Exercises c_gen.cpp BuildLocalVarExtensions line 797 (the continue
-- that skips processing when a top-level local has no explist).
local x

function test()
    return 1
end
