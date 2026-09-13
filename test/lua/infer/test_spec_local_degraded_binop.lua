-- local x 以 binop（n + 1）初始化，之后在 if 分支中被赋值为浮点数 0.5。
-- 后处理将 x 的类型降级为 T_DYNAMIC（MergeType(T_INT, T_FLOAT) = T_DYNAMIC），
-- 并将 current_map_[n+1] 也改写为 T_DYNAMIC。
-- Bug 2：修复前，is_degraded_literal 只覆盖 kNumber 字面量，不覆盖 kBinop；
--   InferArgTypeForSpec(n+1) 仍返回 T_INT，导致 x 被声明为 int64_t，
--   之后 x = 0.5 被静默截断为 0，函数对负数参数返回 0 而非 0.5。
-- 修复后：is_degraded_expression 覆盖 kBinop，x 正确声明为 CVar。
-- test(5) == 6（走 n + 1 路径），test(-1) == 0.5（走 0.5 路径）。
function test(n)
    local x = n + 1
    if n < 0 then
        x = 0.5
    end
    return x
end
