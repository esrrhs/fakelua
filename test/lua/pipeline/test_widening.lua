-- Widening 测试: 循环中 shape 增长超过阈值
-- 期望: 最终退化为 T_DYNAMIC
function test()
    local t = {}
    for i = 1, 100 do
        t["k" .. i] = i
    end
    return t
end
