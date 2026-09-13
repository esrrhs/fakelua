-- Bug 2: 位运算应接受 T_FLOAT 操作数（Lua 5.4 自动将整数浮点转为 int）。
-- 修复前：TypeInferencer 仅当两侧均为 T_INT 时才推断位运算为 T_INT。
-- 修复后：两侧均为数值类型时位运算结果推断为 T_INT。
-- test(7, 3.0) == 3 (7 & 3 = 3)
function test(a, b)
    return a & b
end
