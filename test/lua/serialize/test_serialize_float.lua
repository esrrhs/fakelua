package "SerializeTest"

-- 浮点数往返：整数部分为 0 的小数、负数、大数
function test_float()
    local cases = {1.5, -2.25, 0.0, 3.14159265358979, -100.0625, 1234567.875}
    for _, v in ipairs(cases) do
        local bin = serialize.encode(v)
        if not bin then return 0 end
        local d = serialize.decode(bin)
        -- 浮点比较：允许极小误差
        if d == nil then return 0 end
        if d < v - 1e-9 or d > v + 1e-9 then return 0 end
    end
    return 1
end
