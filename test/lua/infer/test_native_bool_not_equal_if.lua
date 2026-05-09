-- NOT_EQUAL (~=) 用于 if 条件语句中的原生布尔表达式测试。
-- n 是数学参数（通过 n + 1 算术运算触发），~= 0 应生成原生 C 不等比较 (n) != (0)。
-- test(0) == 0, test(1) == 2, test(-1) == 0（-1 + 1 = 0 时返回 n+1=0）。
function test(n)
    if n ~= 0 then
        return n + 1
    end
    return 0
end
