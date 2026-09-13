-- 文件级整数局部常量 N 应生成 static const int64_t，
-- 函数体内以 N 为上界的 for 循环应使用 T_INT 循环变量，
-- 累加结果 sum 应声明为 int64_t（N*(N+1)/2 = 5050）。
local N = 100

function test()
    local sum = 0
    for i = 1, N do
        sum = sum + i
    end
    return sum
end
