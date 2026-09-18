function test_float_key_precise()
    local t = { [0.123456789] = 42 }
    local k = 0.123456789
    if t[k] ~= 42 then return 0 end
    if t[0.123456789] ~= 42 then return 0 end
    t[k] = 99
    if t[0.123456789] ~= 99 then return 0 end
    return 1
end
