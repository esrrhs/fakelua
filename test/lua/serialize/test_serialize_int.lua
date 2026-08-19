package "SerializeTest"

-- 整数往返：正数、负数、零、大整数
function test_int()
    local cases = {0, 1, -1, 127, 128, -128, 255, 16383, 16384, -100000,
                  123456789, -987654321, 2147483647, -2147483648}
    for _, v in ipairs(cases) do
        local bin = serialize.encode(v)
        if not bin then return 0 end
        local d = serialize.decode(bin)
        if d ~= v then return 0 end
    end
    return 1
end
