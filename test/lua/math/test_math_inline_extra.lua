-- Extra math inline / native fallback coverage.
function test_math_inline_extra()
    if math.abs(math.fmod(10, 3) - 1) > 1e-9 then return 1 end
    if math.abs(math.ldexp(2.0, 3) - 16.0) > 1e-9 then return 2 end
    if math.type(7) ~= "integer" then return 3 end
    if math.type(7.5) ~= "float" then return 4 end
    if math.type("x") ~= nil then return 5 end
    if math.tointeger(9.0) ~= 9 then return 6 end
    if math.tointeger(9.2) ~= nil then return 7 end
    if not math.ult(1, 2) then return 8 end
    if math.ult(-1, 1) then return 9 end
    local ok = pcall(function() return math.fmod("a", 1) end)
    if ok then return 10 end
    -- string -> tointeger native fallback
    if math.tointeger("11") ~= 11 then return 11 end
    return 0
end
