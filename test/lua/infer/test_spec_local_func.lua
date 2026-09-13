-- local function 形式的数学参数特化。
-- square 通过 local function 定义，n 是数学参数（n*n 有算术改善）。
-- DiscoverMathParams 对 LocalFunction 和 Function 均适用。
-- test 通过嵌套调用推断也被特化（square(n)+1 触发改善）。
-- test(5) == 26, test(3) == 10.
local function square(n)
    return n * n
end

function test(n)
    return square(n) + 1
end
