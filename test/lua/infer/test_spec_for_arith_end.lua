-- 数学参数 n 作为 for 循环上界的算术表达式（n * 2）。
-- 整数特化中，ExpEnd = n*2 被快照标注为 T_INT，
-- 使 CompileStmtForLoop 走 typed_int_for 路径。
-- test(5) == 1+2+...+10 = 55.
function test(n)
    local sum = 0
    for i = 1, n * 2 do
        sum = sum + i
    end
    return sum
end
