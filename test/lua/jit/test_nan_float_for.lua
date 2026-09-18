-- Lua 5.4 浮点 for：NaN limit 的 FORPREP 用严格 <，NaN 不跳过，循环体执行一次。
function test_nan_float_for()
    local n = 0
    for i = 1.0, 0 / 0 do
        n = n + 1
        if n > 5 then return -1 end
    end
    return n
end

function test_nan_float_for_neg()
    local n = 0
    for i = 1.0, 0 / 0, -1.0 do
        n = n + 1
        if n > 5 then return -1 end
    end
    return n
end

-- 动态上界：运行时走浮点 FORPREP/FORLOOP。
function test_nan_dynamic_float(e)
    local n = 0
    for i = 1.0, e do
        n = n + 1
        if n > 5 then return -1 end
    end
    return n
end

-- init+step 为整数、limit 动态 NaN：走整数 forlimit。
-- 负 step：NaN → MININTEGER，循环会跑；安全阀与 test_nan_for_limit 一致。
function test_nan_dynamic_int_neg(e)
    local n = 0
    for i = 1, e, -1 do
        n = n + 1
        if n > 2 then return n end
    end
    return n
end

-- 正 step：NaN → 整段跳过。
function test_nan_dynamic_int_pos(e)
    local n = 0
    for i = 1, e do
        n = n + 1
        if n > 5 then return -1 end
    end
    return n
end
