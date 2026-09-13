-- Lua 5.4 语义：移位量 >= 64 时结果为 0（逻辑移位）
function test(a, shift)
    return a >> shift
end
