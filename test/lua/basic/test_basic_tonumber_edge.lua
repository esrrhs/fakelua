-- Test tonumber edge cases
-- Covers: whitespace, leading zeros, decimal notation, base boundaries

function test_basic_tonumber_edge()
    -- -------------------------------------------------------------------------
    -- Whitespace handling
    -- -------------------------------------------------------------------------
    -- Leading/trailing spaces and tabs
    if tonumber("   42   ") ~= 42 then return 20 end
    if tonumber("\t100\t") ~= 100 then return 21 end
    if tonumber("\n200\n") ~= 200 then return 22 end
    -- Mixed whitespace
    if tonumber("  \t  300  \t  ") ~= 300 then return 23 end

    -- -------------------------------------------------------------------------
    -- Leading zeros
    -- -------------------------------------------------------------------------
    if tonumber("00000123") ~= 123 then return 30 end
    if tonumber("-00000123") ~= -123 then return 31 end
    if tonumber("00000") ~= 0 then return 32 end
    if tonumber("-00000") ~= 0 then return 33 end  -- -0 = 0

    -- -------------------------------------------------------------------------
    -- Decimal notation edge cases
    -- -------------------------------------------------------------------------
    -- Trailing decimal point
    local trailing_dot = tonumber("42.")
    if trailing_dot ~= nil and trailing_dot ~= 42 then return 40 end
    -- Leading decimal point
    local leading_dot = tonumber(".5")
    if leading_dot ~= nil and math.abs(leading_dot - 0.5) > 0.001 then return 41 end
    -- Leading decimal point with sign
    local neg_dot = tonumber("-.5")
    if neg_dot ~= nil and math.abs(neg_dot - (-0.5)) > 0.001 then return 42 end
    local pos_dot = tonumber("+.5")
    if pos_dot ~= nil and math.abs(pos_dot - 0.5) > 0.001 then return 43 end

    -- -------------------------------------------------------------------------
    -- Base parameter edge cases
    -- -------------------------------------------------------------------------
    -- Base 2: only 0 and 1 are valid digits
    if tonumber("10", 2) ~= 2 then return 50 end
    if tonumber("11", 2) ~= 3 then return 51 end
    -- Characters outside digit range for base 2
    if tonumber("12", 2) ~= nil then return 52 end
    if tonumber("1a", 2) ~= nil then return 53 end

    -- Base 36: all alphanumeric are valid digits
    if tonumber("z", 36) ~= 35 then return 54 end
    if tonumber("10", 36) ~= 36 then return 55 end
    if tonumber("1z", 36) ~= 71 then return 56 end  -- 1*36 + 35 = 71

    -- Base out of range returns nil
    if tonumber("123", 0) ~= nil then return 57 end
    if tonumber("123", 1) ~= nil then return 58 end
    if tonumber("123", 37) ~= nil then return 59 end
    if tonumber("123", -1) ~= nil then return 60 end

    -- -------------------------------------------------------------------------
    -- Already-number inputs
    -- -------------------------------------------------------------------------
    if tonumber(42) ~= 42 then return 70 end
    if math.abs(tonumber(3.14) - 3.14) > 0.001 then return 71 end

    -- -------------------------------------------------------------------------
    -- Large hex that fits in int64 range
    -- -------------------------------------------------------------------------
    -- Standard test only covers small hex (0x10); test a larger but valid hex
    if tonumber("0x1000") ~= 4096 then return 72 end
    if tonumber("-0x1000") ~= -4096 then return 73 end

    -- -------------------------------------------------------------------------
    -- Invalid/empty strings
    -- -------------------------------------------------------------------------
    if tonumber("") ~= nil then return 80 end
    if tonumber("   ") ~= nil then return 81 end  -- whitespace only
    if tonumber("abc") ~= nil then return 82 end
    if tonumber("--42") ~= nil then return 83 end

    return 5000
end
