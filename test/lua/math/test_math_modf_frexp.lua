function test_math_modf_frexp()
    local i, f = math.modf(3.14)
    if not (i == 3.0 and math.abs(f - 0.14) < 1e-4) then return 0 end

    local frac, exp = math.frexp(8.0)
    if not (frac == 0.5 and exp == 4) then return 0 end
    return 400
end
