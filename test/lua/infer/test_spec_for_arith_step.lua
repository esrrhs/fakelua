-- 数学参数 n 作为 for 循环步长的算术表达式（n + 1）。
-- 整数特化中，ExpStep = n+1 被快照标注为 T_INT，
-- 使 CompileStmtForLoop 走 typed_int_for 路径（begin/end/step 全为 T_INT）。
-- test(2): i = 1, 4, 7, 10  ->  sum = 22.
-- test(3): i = 1, 5, 9      ->  sum = 15.
function test(n)
    local sum = 0
    for i = 1, 10, n + 1 do
        sum = sum + i
    end
    return sum
end
