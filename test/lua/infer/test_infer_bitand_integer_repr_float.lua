-- 位运算应允许可表示为整数的浮点数（如 1.0），
-- 并拒绝非整数浮点数（如 1.5）。
function test(n)
    return n & 3
end

function test_err()
    return test(1.5)
end
