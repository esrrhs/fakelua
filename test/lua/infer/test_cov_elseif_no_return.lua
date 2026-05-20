-- Function with if-elseif where the elseif block does NOT return.
-- Exercises AllPathsReturn lines 860-861: when checking elseif blocks, one
-- block lacks a return so AllPathsReturn returns false for that block.
-- The last statement of the function body is the if-elseif-else, so
-- AllPathsReturn recurses into the elseif block and triggers line 861.
-- test(15) == 30 (n > 10 path), test(3) == 3 (else path).
function test(n)
    if n > 10 then
        return n * 2
    elseif n > 5 then
        local x = n + 1  -- no return in this branch
    else
        return n
    end
end
