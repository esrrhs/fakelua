-- ForLoop where the end bound n is a math parameter (int64_t or double).
-- int specialization: all bounds T_INT → int64_t loop vars, int64_t sum.
-- float specialization: end bound becomes T_FLOAT → double loop ctrl vars,
-- sum degrades to CVar because MergeType(T_INT, T_FLOAT) = T_DYNAMIC.
-- test(10) -> sum(1..10) = 55.
function test(n)
    local sum = 0
    for i = 1, n do
        sum = sum + i
    end
    return sum
end
