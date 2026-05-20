-- 覆盖 HasForLoopTypeChange line 821：
-- 在 all_int 试推断中，for 循环变量因循环体内被赋值为 T_DYNAMIC（函数调用结果）
-- 而最终类型变为 T_DYNAMIC，触发 !IsNumericInferredType 检查（line 821）。
-- getVal() 返回 T_DYNAMIC；test 的 n*2 使 n 成为数学参数。
function getVal()
    return 0
end

function test(n)
    for i = 1, n do
        i = getVal()
    end
    return n * 2
end
