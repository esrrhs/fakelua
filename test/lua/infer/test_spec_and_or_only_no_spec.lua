-- 仅含 and/or 运算符（无算术、无有序比较）的函数不应被特化。
-- a 和 b 仅用于 and/or，可接受任意 Lua 类型（nil、字符串等），
-- 因此 ParamAffectsArithmetic 对两者均返回 false，不会生成特化版本。
-- test(1, 2) == 2 (1 and 2 = 2), test(0, 2) == 2 (0 and 2 = 2),
-- test(nil, 2) == nil (nil and 2 = nil... wait, nil is falsy so nil and 2 = nil).
-- Actually in Lua: nil and 2 = nil, 0 and 2 = 2 (0 is truthy), 1 and 2 = 2.
local function test(a, b)
    return a and b
end
