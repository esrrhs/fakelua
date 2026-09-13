-- ForLoop where the step is a math parameter (int64_t or double specializations).
-- int specialization: begin/end/step all T_INT → int64_t loop vars, int64_t sum.
-- float specialization: step becomes T_FLOAT → double loop ctrl vars, sum degrades
-- to CVar because MergeType(T_INT, T_FLOAT) = T_DYNAMIC.
-- With step = 2:  i = 1, 3, 5, 7, 9  →  sum = 25.
function test(step)
    local sum = 0
    for i = 1, 9, step do
        sum = sum + i
    end
    return sum
end
