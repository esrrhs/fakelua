-- 覆盖 HasMathCallImprovement line 720：
-- 数学函数体内以零参数调用另一个数学函数时，raw_args 为空 → 提前返回（line 720）。
-- compute 是数学函数（return n*2），test 也是数学函数（return n*2），
-- test 体内调用 compute()（无参数），触发 raw_args.empty() 检查。
function compute(n)
    return n * 2
end

function test(n)
    compute()
    return n * 2
end
