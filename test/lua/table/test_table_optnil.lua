-- 可选参数显式传 nil 必须与省略等价（Lua 5.4 luaL_optinteger/luaL_optlstring 语义）。
-- 覆盖包装函数少传实参时形参以显式 nil 进入被调用库函数的常见转发写法。
local function concat_wrap(t, sep, i, j)
    return table.concat(t, sep, i, j)
end

local function unpack_wrap(t, i, j)
    local a, b, c = table.unpack(t, i, j)
    return a + b + c
end

function test_table_optnil()
    local total = 0
    local t = { 1, 2, 3, 4, 5 }

    -- table.concat：i 默认 1，j 默认 #t
    total = total + #concat_wrap(t, "-", nil, nil)  -- "1-2-3-4-5" = 9
    total = total + #concat_wrap(t, "-", 2, nil)    -- "2-3-4-5"   = 7
    total = total + #concat_wrap(t, "-", nil, 3)    -- "1-2-3"     = 5
    total = total + #table.concat(t, "-", nil, nil) -- 同上直接调用 = 9

    -- table.concat：sep 显式 nil 与省略等价，默认 ""（luaL_optlstring 语义）
    total = total + #concat_wrap(t, nil, nil, nil)  -- "12345" = 5
    total = total + #table.concat(t, nil, 2, 4)     -- "234" = 3

    -- table.unpack：i 默认 1，j 默认 #t（取前三个值相加）
    total = total + unpack_wrap(t, nil, nil)        -- 1+2+3 = 6
    total = total + unpack_wrap(t, 2, nil)          -- 2+3+4 = 9

    -- table.remove：pos 省略/nil 取 #t
    local r = { 10, 20, 30 }
    total = total + table.remove(r, nil)          -- 30

    return total
end
