-- 特化表字面量键：十六进制、1.0 归一成整数、显式键不推进隐式下标、转义、pairs 可见赋值

function test_hex_key()
    local t = { [0x10] = 16, [0x20] = 32 }
    return t[16] + t[32]
end

function test_float_one_is_int()
    local t = { [1.0] = 7 }
    return t[1]
end

function test_implicit_after_explicit()
    -- Lua：隐式数组下标从 1 独立递增，显式 [2] 不占用隐式计数
    local t = { [2] = 20, 10 }
    if t[1] ~= 10 then return 0 end
    if t[2] ~= 20 then return 0 end
    return 1
end

function test_quote_key()
    local t = { ["a\"b"] = 9 }
    return t["a\"b"]
end

function test_spec_assign_pairs()
    local t = { a = 1 }
    t.b = 2
    local n = 0
    local sum = 0
    for k, v in pairs(t) do
        n = n + 1
        sum = sum + v
    end
    if n ~= 2 then return 0 end
    if sum ~= 3 then return 0 end
    return 1
end
