-- 顶层 "local square = function(n) ... end" 应与 "local function square(n) ... end" 等价，
-- 在预处理阶段被转换为 LocalFunction，从而参与数学参数特化流程。
-- square(n) 含 n*n 算术运算，n 是数学参数；test(n) 调用 square(n) 传递特化。
-- test(3) == 9, test(4) == 16, test(2.0) == 4.0.
local square = function(n)
    return n * n
end

function test(n)
    return square(n)
end
