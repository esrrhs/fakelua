-- 仅含按位取反的函数：function f(n) return ~n end
-- IsArithmeticExpr 现在也识别 unop BITNOT，使 n 被检测为数学参数。
-- 整型特化：~T_INT = T_INT，直接发射 ~((int64_t)(n))。
-- 浮点特化：~T_FLOAT = T_DYNAMIC（按位取反对浮点数无意义），特化体返回 CVar。
-- test(5) == -6（~5 = -6 in two's complement）。
function test(n)
    return ~n
end
