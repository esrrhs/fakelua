-- 数学参数 n 作为 for 循环起始表达式（n + 1）。
-- 整数特化中，ExpBegin = n+1 被快照标注为 T_INT，
-- 使 CompileStmtForLoop 走 typed_int_for 路径（begin/end 全为 T_INT）。
-- test(10) == 11+12+...+20 = 155
-- test(0)  == 1+2+...+20  = 210
-- test(19) == 20           = 20
function test(n)
    local sum = 0
    for i = n + 1, 20 do
        sum = sum + i
    end
    return sum
end
