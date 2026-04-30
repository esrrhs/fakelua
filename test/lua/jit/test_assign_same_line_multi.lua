-- Tests that two multi-assign statements on the same source line do not collide
-- on temp variable names during pre-processing.
function test()
    local a = 0
    local b = 0
    local c = 0
    local d = 0
    a, b = 1, 2; c, d = 3, 4
    return a + b + c + d
end
