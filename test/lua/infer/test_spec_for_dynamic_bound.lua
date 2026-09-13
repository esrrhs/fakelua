-- mul2 is a math function: n is a math param (n*2 arithmetic).
-- iter only passes n to mul2() as the for-loop upper bound.
-- Because iter has no arithmetic of its own, the for-loop type remains
-- T_DYNAMIC in trial inference (mul2(n) returns T_DYNAMIC without hints).
-- HasForLoopTypeChange skips that for-loop (non-numeric in typed map) and
-- HasMathCallImprovement detects the improvement via the mul2(n) call arg.
-- This exercises the !IsNumericInferredType early-return in HasForLoopTypeChange.
local function mul2(n)
    return n * 2
end

local function iter(n)
    local sum = 0
    for i = 1, mul2(n) do
        sum = sum + i
    end
    return sum
end

function test(n)
    return iter(n)
end
