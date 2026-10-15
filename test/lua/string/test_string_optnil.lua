-- 可选参数显式传 nil 必须与省略等价（Lua 5.4 luaL_optinteger/luaL_optlstring 语义）。
-- 注意逐函数差异（以 5.4.9 源码为准）：
--   string.sub   i 是 checkinteger（nil 报错），j 是 optinteger（nil 取默认 -1）
--   string.byte  i/j 均 optinteger；string.rep sep 是 optlstring（nil 取 ""）
--   string.find/match init 是 optinteger；string.gsub n 缺省/nil 不限次数
local function sub_wrap(s, i, j)
    return #string.sub(s, i, j)
end

local function byte_wrap(s, i, j)
    local a, b = string.byte(s, i, j)
    return (a or 0) + (b or 0)
end

local function rep_wrap(s, n, sep)
    return #string.rep(s, n, sep)
end

local function find_wrap(s, pat, init)
    local a, b = string.find(s, pat, init)
    if a == nil then return 0 end
    return a * 100 + b
end

local function match_wrap(s, pat, init)
    local m = string.match(s, pat, init)
    if m == nil then return 0 end
    return #m
end

function test_string_optnil()
    local total = 0

    -- string.sub：j 传 nil 取默认（末尾）；i 传 nil 必须报错（与 Lua 一致）
    total = total + sub_wrap("hello", 2, nil)     -- "ello" = 4
    total = total + sub_wrap("hello", 2, 4)       -- "ell"  = 3
    local ok = pcall(function() string.sub("hello", nil) end)
    if ok then total = total + 1000 end            -- 不应发生：i 为 nil 必须报错

    -- string.byte：i 默认 1，j 默认 i（j 给定时返回两个字节）
    total = total + byte_wrap("hello", nil, nil)   -- 104
    total = total + byte_wrap("hello", 2, nil)    -- 101
    total = total + byte_wrap("hello", nil, 2)    -- 104+101 = 205

    -- string.rep：sep 传 nil 取默认 ""
    total = total + rep_wrap("ab", 3, nil)        -- "ababab"   = 6
    total = total + rep_wrap("ab", 3, "-")        -- "ab-ab-ab" = 8

    -- string.find/match：init 默认 1
    total = total + find_wrap("hello", "l", nil)  -- 3,3 -> 303
    total = total + find_wrap("hello", "l", 4)    -- 4,4 -> 404
    total = total + match_wrap("hello", "l+", nil)-- "ll" = 2
    total = total + match_wrap("hello", "xyz", nil)-- 0

    -- string.gsub：n 缺省/nil 不限次数
    local s, cnt = string.gsub("aaa", "a", "b", nil)
    total = total + #s + cnt                      -- "bbb" = 3 + 3

    return total
end
