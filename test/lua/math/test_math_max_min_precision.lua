function test_math_max_min_precision()
    local positive_int = 9007199254740993
    local positive_float = 9007199254740992.0
    if math.max(positive_float, positive_int) ~= positive_int then return 1 end
    if math.type(math.max(positive_float, positive_int)) ~= "integer" then return 2 end
    if math.min(positive_int, positive_float) ~= positive_float then return 3 end
    if math.type(math.min(positive_int, positive_float)) ~= "float" then return 4 end

    local negative_int = -9007199254740993
    local negative_float = -9007199254740992.0
    if math.max(negative_int, negative_float) ~= negative_float then return 5 end
    if math.type(math.max(negative_int, negative_float)) ~= "float" then return 6 end
    if math.min(negative_float, negative_int) ~= negative_int then return 7 end
    if math.type(math.min(negative_float, negative_int)) ~= "integer" then return 8 end

    return 5000
end

function test_math_max_invalid_string()
    local ok = pcall(math.max, "not a number")
    return ok and 0 or 5000
end

function test_math_min_invalid_string()
    local ok = pcall(math.min, 1, "not a number")
    return ok and 0 or 5000
end

function test_math_other_edge_cases()
    local ok = pcall(function() return math.sqrt("not a number") end)
    if ok then return 11 end

    ok = pcall(function() math.randomseed(1e308) end)
    if ok then return 12 end
    ok = pcall(function() math.randomseed(1.5) end)
    if ok then return 13 end
    if not pcall(function() math.randomseed("2") end) then return 14 end
    if not pcall(function() math.random("2") end) then return 15 end

    if math.ldexp(1.0, 2147483648) ~= math.huge then return 16 end
    if math.ldexp(1.0, -2147483649) ~= 0.0 then return 17 end

    math.randomseed(42)
    for _ = 1, 8 do
        if math.random(math.mininteger, math.maxinteger) == math.mininteger then
            return 18
        end
    end
    return 5000
end
