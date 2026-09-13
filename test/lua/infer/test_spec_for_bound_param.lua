-- Math param n: used as the upper bound of a for loop.
-- In the int specialization, CompileStmtForLoop detects all bounds are T_INT
-- (from the specialization snapshot) and uses the native int64_t fast path.
-- The inner sum = sum + i is T_INT + T_INT, confirming arithmetic improvement.
-- test(10) == 55 (1+2+...+10), test(5) == 15 (1+2+3+4+5).
function test(n)
    local sum = 0
    for i = 1, n do
        sum = sum + i
    end
    return sum
end
