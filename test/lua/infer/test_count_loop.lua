-- Count iterations from 1 to n (n is the for-loop end bound).
-- The loop variable i is NOT used in any arithmetic inside the body.
-- Exercises: math param specialization via ForLoop EvalType improvement
-- when the loop variable does not appear in body arithmetic.
local function count_to(n)
    local c = 0
    for i = 1, n do
        c = c + 1
    end
    return c
end

-- Count iterations from n down to 1 (n is the for-loop start bound).
-- The loop variable i is NOT used in any arithmetic inside the body.
local function count_from(n)
    local c = 0
    for i = n, 1, -1 do
        c = c + 1
    end
    return c
end

-- Count iterations with n as the for-loop step (end bound is fixed).
-- The loop variable i is NOT used in any arithmetic inside the body.
local function count_step(n)
    local c = 0
    for i = 0, 100, n do
        c = c + 1
    end
    return c
end

function test_count_to(n)
    return count_to(n)
end

function test_count_from(n)
    return count_from(n)
end

function test_count_step(n)
    return count_step(n)
end
