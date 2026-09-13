-- 数学参数 n 用作 for 循环的起始值（begin）。
-- 整数特化中，ExpBegin = n 被快照标注为 T_INT，
-- 使 CompileStmtForLoop 走 typed_int_for 路径，控制变量使用原生 int64_t。
-- test(3)  == 3+4+...+10 = 52
-- test(8)  == 8+9+10 = 27
-- test(11) == 0（空区间）
function test(n)
    local sum = 0
    for i = n, 10 do
        sum = sum + i
    end
    return sum
end
