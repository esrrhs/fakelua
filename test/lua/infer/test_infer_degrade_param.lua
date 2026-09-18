-- ForLoop where the end bound n is a math parameter (int64_t or double).
-- int specialization: all bounds T_INT → int64_t loop vars, int64_t sum.
-- float specialization: Lua 5.4 整数 for（init/step 为整数），limit 走 FlForLimitToInt，
-- 循环变量仍是 int64_t，sum 保持 int64_t。
-- test(10) -> sum(1..10) = 55.
function test(n)
    local sum = 0
    for i = 1, n do
        sum = sum + i
    end
    return sum
end
