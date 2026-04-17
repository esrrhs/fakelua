-- Type inference mid-way degradation: loop bound n is a parameter (T_DYNAMIC),
-- so the loop variable i degrades to T_DYNAMIC, which in turn causes sum to
-- degrade from T_INT to T_DYNAMIC.  Result must still be correct.
-- test(10) -> sum(1..10) = 55.
function test(n)
    local sum = 0
    for i = 1, n do
        sum = sum + i
    end
    return sum
end
