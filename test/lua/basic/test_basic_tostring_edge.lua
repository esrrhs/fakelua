-- Test tostring edge cases
-- Covers: large integers, float precision, special numeric values

function test_basic_tostring_edge()
    -- -------------------------------------------------------------------------
    -- Large integers
    -- -------------------------------------------------------------------------
    -- Near math.maxinteger
    local maxi = math.maxinteger
    local maxi_str = tostring(maxi)
    -- Must not return nil and must be a non-empty string
    if maxi_str == nil then return 1 end
    if #maxi_str == 0 then return 2 end

    -- Near math.mininteger
    local mini = math.mininteger
    local mini_str = tostring(mini)
    if mini_str == nil then return 3 end
    if #mini_str == 0 then return 4 end
    -- Should start with '-'
    if string.sub(mini_str, 1, 1) ~= "-" then return 5 end

    -- Zero
    if tostring(0) ~= "0" then return 6 end
    if tostring(-0) ~= "0" then return 6.1 end  -- Lua normalizes -0 to 0 in decimal

    -- -------------------------------------------------------------------------
    -- Standard integer formatting
    -- -------------------------------------------------------------------------
    if tostring(12345) ~= "12345" then return 10 end
    if tostring(-100) ~= "-100" then return 11 end

    -- -------------------------------------------------------------------------
    -- Float values
    -- -------------------------------------------------------------------------
    -- Simple float
    local pi_str = tostring(3.14)
    if pi_str == nil or #pi_str == 0 then return 20 end

    -- Very small float (may use scientific notation)
    local small_str = tostring(0.0001)
    if small_str == nil or #small_str == 0 then return 21 end

    -- Large float
    local large_str = tostring(1e20)
    if large_str == nil or #large_str == 0 then return 22 end

    -- -------------------------------------------------------------------------
    -- Boolean and nil (already covered in basic, but verify consistency)
    -- -------------------------------------------------------------------------
    if tostring(true) ~= "true" then return 30 end
    if tostring(false) ~= "false" then return 31 end
    if tostring(nil) ~= "nil" then return 32 end

    -- -------------------------------------------------------------------------
    -- String passthrough
    -- -------------------------------------------------------------------------
    if tostring("hello") ~= "hello" then return 40 end
    if tostring("") ~= "" then return 41 end

    -- -------------------------------------------------------------------------
    -- Conversion roundtrip: tostring → tonumber
    -- -------------------------------------------------------------------------
    -- Integer roundtrip
    local orig = 42
    local round = tonumber(tostring(orig))
    if round ~= orig then return 50 end

    -- Negative integer roundtrip
    local orig2 = -12345
    local round2 = tonumber(tostring(orig2))
    if round2 ~= orig2 then return 51 end

    -- Float roundtrip (may lose precision, so check approximate)
    local orig3 = 3.14
    local round3 = tonumber(tostring(orig3))
    if round3 == nil then return 52 end
    -- Float roundtrip may not be exact, just verify it's close
    if math.abs(round3 - orig3) > 0.1 then return 53 end

    -- Large integer roundtrip
    local orig4 = 1234567890
    local round4 = tonumber(tostring(orig4))
    if round4 ~= orig4 then return 54 end

    return 5000
end
