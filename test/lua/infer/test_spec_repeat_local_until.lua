-- repeat...until 中，until 条件引用了块内声明的局部变量 step。
-- Lua 语义：until 条件可见 repeat 块内的 local 声明。
-- Bug 1（编译器）：修复前，NativeVarScope 在块退出时被清除，
--   导致 until 条件中的 step 被当作 CVar 而非 int64_t 处理，引发 C 编译错误。
-- Bug 3（类型推断）：修复前，类型推断器在推断 until 条件之前退出了作用域，
--   导致 step 被推断为 T_DYNAMIC 而非 T_INT，阻止了原生比较的生成。
-- 循环逻辑：step = n，sum += step，until step >= n → 第一次迭代后立即退出。
-- test(5) == 5，test(3) == 3，test(10) == 10。
function test(n)
    local sum = 0
    repeat
        local step = n
        sum = sum + step
    until step >= n
    return sum
end
