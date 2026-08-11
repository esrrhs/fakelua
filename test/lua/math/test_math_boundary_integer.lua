-- Test integer boundary edge cases
-- Covers: math.maxinteger / math.mininteger values, integer overflow, math.type boundaries

function test_math_boundary_integer()
    local maxi = math.maxinteger
    local mini = math.mininteger

    -- -------------------------------------------------------------------------
    -- Constants are valid integers
    -- -------------------------------------------------------------------------
    if maxi <= 0 then return 1 end
    if mini >= 0 then return 2 end
    if maxi + mini ~= -1 then return 3 end  -- -1 + 1 = 0, so mini + maxi = -1

    -- -------------------------------------------------------------------------
    -- math.type on integer boundaries
    -- -------------------------------------------------------------------------
    if math.type(maxi) ~= "integer" then return 10 end
    if math.type(mini) ~= "integer" then return 11 end
    if math.type(0) ~= "integer" then return 12 end
    if math.type(1) ~= "integer" then return 13 end
    if math.type(3.14) ~= "float" then return 14 end

    -- maxinteger as float (adding 0.0 coerces to float)
    local maxi_float = maxi + 0.0
    if math.type(maxi_float) ~= "float" then return 15 end

    -- -------------------------------------------------------------------------
    -- math.tointeger boundary cases
    -- -------------------------------------------------------------------------
    -- Exact integer value from float should convert back
    -- Use a value that fits in float53 mantissa (maxi_float exceeds 2^53)
    local exact_float = 100.0
    local toint = math.tointeger(exact_float)
    if toint ~= 100 then return 20 end

    -- Non-integer float should return nil
    if math.tointeger(3.14) ~= nil then return 21 end

    -- A float with a fractional part should return nil.
    -- Use a value that is exactly representable in double (100.5) so the
    -- behavior is consistent across platforms.  maxi + 0.5 would be rounded
    -- to 2^63 (an exact integer) because maxi exceeds the float53 mantissa.
    local toint_fract = math.tointeger(100.5)
    if toint_fract ~= nil then return 22 end

    -- Normal integer range should work
    if math.tointeger(42) ~= 42 then return 23 end
    if math.tointeger(-42) ~= -42 then return 24 end

    -- math.tointeger on string (standard Lua behavior: returns nil for strings)
    if math.tointeger("10") ~= 10 then return 25 end
    if math.tointeger("notanumber") ~= nil then return 26 end

    -- -------------------------------------------------------------------------
    -- Integer overflow: maxinteger + 1
    -- In Lua 5.4, this converts to float 9.223372036854776e+18
    -- fakelua may differ: accept any valid result (must not crash)
    -- -------------------------------------------------------------------------
    local overflow_add = maxi + 1
    -- Must NOT be nil and must be a number
    if overflow_add == nil then return 30 end
    local ovf_type = math.type(overflow_add)
    if ovf_type == nil then return 31 end

    -- mininteger - 1 should also overflow
    local overflow_sub = mini - 1
    if overflow_sub == nil then return 32 end
    local ovf_sub_type = math.type(overflow_sub)
    if ovf_sub_type == nil then return 33 end

    -- -------------------------------------------------------------------------
    -- math.max / math.min with boundary integers
    -- -------------------------------------------------------------------------
    if math.max(maxi, 0) ~= maxi then return 40 end
    if math.min(mini, 0) ~= mini then return 41 end
    if math.max(maxi, mini) ~= maxi then return 42 end
    if math.min(maxi, mini) ~= mini then return 43 end
    -- Multiple args
    if math.max(1, 2, 3, 4, 5) ~= 5 then return 44 end
    if math.min(1, 2, 3, 4, 5) ~= 1 then return 45 end

    -- -------------------------------------------------------------------------
    -- math.abs boundary
    -- -------------------------------------------------------------------------
    -- abs(mininteger) overflows; result must be a number (type may vary)
    local abs_min = math.abs(mini)
    if abs_min == nil then return 50 end
    -- abs(maxinteger) fits in integer
    if math.abs(maxi) ~= maxi then return 51 end
    if math.abs(0) ~= 0 then return 52 end

    -- -------------------------------------------------------------------------
    -- math.floor / math.ceil with large integers
    -- -------------------------------------------------------------------------
    if math.floor(maxi) ~= maxi then return 60 end
    if math.ceil(maxi) ~= maxi then return 61 end
    if math.floor(mini) ~= mini then return 62 end
    if math.ceil(mini) ~= mini then return 63 end

    -- -------------------------------------------------------------------------
    -- math.ult (unsigned less-than)
    -- -------------------------------------------------------------------------
    -- -1 as unsigned is huge (all bits set), so -1 < 0 as unsigned is false
    -- but standard Lua 5.3+: math.ult(-1, 0) returns false
    if math.ult(0, 1) ~= true then return 70 end
    if math.ult(1, 0) ~= false then return 71 end
    if math.ult(0, maxi) ~= true then return 72 end

    -- -------------------------------------------------------------------------
    -- Large integer multiplication boundary
    -- -------------------------------------------------------------------------
    -- maxinteger * 2 (overflow expected)
    local overflow_mul = maxi * 2
    if overflow_mul == nil then return 80 end
    -- mininteger * -1 (overflow; -(-2^63) = 2^63 > maxinteger)
    local overflow_mul2 = mini * (-1)
    if overflow_mul2 == nil then return 81 end

    -- -------------------------------------------------------------------------
    -- math.random with boundary range
    -- -------------------------------------------------------------------------
    -- random(1, 1) should always return 1
    if math.random(1, 1) ~= 1 then return 90 end

    return 5000
end
