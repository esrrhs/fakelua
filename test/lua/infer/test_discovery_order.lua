-- Bug 4: 数学参数发现顺序依赖。
-- inner 是数学函数（包含算术运算），outer 通过调用 inner 间接参与算术。
-- 修复前：若 outer 在 inner 之前被处理，outer 的参数不会被检测为数学参数
-- （因为处理 outer 时 inner 还不在 math_param_positions 中）。
-- 修复后：多轮迭代使 outer 的参数也被正确识别。
-- outer(3) == 7 (inner(3) + 1 = 6 + 1 = 7)
function outer(n)
    return inner(n) + 1
end

function inner(n)
    return n + n
end
