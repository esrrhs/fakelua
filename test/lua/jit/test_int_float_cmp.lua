-- Lua 5.4：|i|>2^53 时 int 与 float 不能先把 int 转成 double 再比。
-- maxinteger + 0.0 是 2^63，与 maxinteger 不相等，且 maxinteger < 2^63。

function test_int_float_cmp()
    local i = math.maxinteger
    local f = i + 0.0
    if i == f then return 1 end
    if not (i < f) then return 2 end
    if f < i then return 3 end
    if not (i <= f) then return 4 end
    if f <= i then return 5 end
    if not (1 == 1.0) then return 6 end
    if not (1 < 1.5) then return 7 end
    if 1 < 0.5 then return 8 end
    if not (2.0 == 2) then return 9 end
    if not (2 > 1.5) then return 10 end
    return 100
end

local function spec_eq(n)
    return n == n + 0.0
end

local function spec_lt(n)
    return n < n + 0.0
end

function test_int_float_cmp_spec()
    if spec_eq(1) ~= true then return 1 end
    if spec_eq(math.maxinteger) ~= false then return 2 end
    if spec_lt(1) ~= false then return 3 end
    if spec_lt(math.maxinteger) ~= true then return 4 end
    return 100
end
