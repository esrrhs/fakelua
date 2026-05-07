-- Specializable function with elseif where not all paths explicitly return
-- via AllPathsReturn. Exercises type_inferencer.cpp lines 804-805
-- (AllPathsReturn returning false when elseif block does not return).
-- test(15) = 30, test(8) = 9, test(3) = 3.
function test(n)
    if n > 10 then
        return n * 2
    elseif n > 5 then
        return n + 1
    end
    return n
end
