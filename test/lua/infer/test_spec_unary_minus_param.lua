-- 仅含一元负号的函数：function f(n) return -n end
-- IsArithmeticExpr 现在也识别 unop MINUS，使 n 被检测为数学参数并生成特化版本。
-- 整型特化返回 int64_t，浮点特化返回 double。
-- f(5) == -5，f(-3) == 3。
function test(n)
    return -n
end
