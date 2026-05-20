-- 覆盖 HasMathCallImprovement line 720：
-- 数学函数体内以零参数调用另一个数学函数时，raw_args 为空 → 提前返回（line 720）。
-- 同时验证代码生成器 nil 补参修复：compute() 缺少参数时，
-- 生成的 C 代码应补充 kNil，即 compute(kNil)，而不是无效的 compute()。
function compute(n)
    return n * 2
end

function test(n)
    compute()
    return n * 2
end
