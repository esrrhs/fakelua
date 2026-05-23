-- 文件级浮点局部常量 PI 应生成 static const double，
-- 函数体内使用 PI 的算术表达式应为原生 double 运算。
local PI = 3.14159

function test(r)
    return PI * r * r
end
