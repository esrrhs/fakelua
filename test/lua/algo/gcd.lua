-- GCD via Euclidean algorithm (recursive) and derived LCM.
-- Exercises: recursion, modulo, if/else, integer arithmetic.
local function gcd(a, b)
    if b == 0 then
        return a
    else
        return gcd(b, a % b)
    end
end

local function lcm(a, b)
    return (a // gcd(a, b)) * b
end

function test_gcd(a, b)
    return gcd(a, b)
end

function test_lcm(a, b)
    return lcm(a, b)
end

-- Count the total number of GCD steps taken (iterative version for variety).
-- Exercises: while loop, multiple local variables, counter.
function test_gcd_steps(a, b)
    local steps = 0
    while b ~= 0 do
        local tmp = b
        b = a % b
        a = tmp
        steps = steps + 1
    end
    return steps
end
