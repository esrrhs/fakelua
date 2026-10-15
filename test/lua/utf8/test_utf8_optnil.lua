-- 可选参数显式传 nil 必须与省略等价（Lua 5.4 luaL_optinteger 语义）。
-- utf8.codepoint/len 的 i/j、utf8.offset 的 i 均为 optinteger；
-- utf8.offset 的 n 是 checkinteger（nil 报错，与 Lua 一致）。
-- "héllo" 的字节布局：h(1) é(2-3) l(4) l(5) o(6)，共 6 字节 5 字符。
local function codepoint_wrap(s, i, j)
    local a, b = utf8.codepoint(s, i, j)
    return (a or 0) + (b or 0)
end

local function len_wrap(s, i, j)
    local n = utf8.len(s, i, j)
    if n == nil then return -1 end
    return n
end

local function offset_wrap(s, n, i)
    return utf8.offset(s, n, i)
end

function test_utf8_optnil()
    local total = 0
    local s = "héllo"

    -- utf8.len：i 默认 1，j 默认 -1（末尾）
    total = total + len_wrap(s, nil, nil)   -- 5
    total = total + len_wrap(s, 2, nil)     -- 从第 2 字节起：é,l,l,o = 4
    total = total + len_wrap(s, nil, 6)     -- 到第 6 字节 = 5
    total = total + len_wrap(s, 3, 6)       -- 第 3 字节是 é 的续字节（非法）-> -1

    -- utf8.codepoint：i 默认 1，j 默认 i（j>i 时返回多个码点，取前两个相加）
    total = total + codepoint_wrap(s, nil, nil)  -- 104 (h)
    total = total + codepoint_wrap(s, 2, nil)    -- 233 (é)
    total = total + codepoint_wrap(s, nil, 3)    -- 104+233 = 337

    -- utf8.offset：n 必填；i 默认 1（n>=0）
    total = total + offset_wrap(s, 2, nil)  -- 第 2 字符起始字节 -> 2
    total = total + offset_wrap(s, 1, nil)  -- 1
    total = total + offset_wrap(s, 0, nil)  -- 1

    -- n 传 nil 必须报错（checkinteger），不能静默取默认
    local ok = pcall(function() utf8.offset(s, nil) end)
    if ok then total = total + 1000 end      -- 不应发生

    return total
end
