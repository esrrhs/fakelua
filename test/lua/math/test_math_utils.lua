function test_math_utils()
    local fm = math.fmod(5.5, 2.0)
    local ld = math.ldexp(1.5, 3)
    local t_int = (math.type(10) == "integer") and 1.0 or 0.0
    local t_flt = (math.type(10.5) == "float") and 1.0 or 0.0
    local to_i = math.tointeger(15.0) or 0
    local ult_res = math.ult(10, 20) and 1.0 or 0.0
    local max_i = (math.maxinteger > 0) and 1.0 or 0.0
    local min_i = (math.mininteger < 0) and 1.0 or 0.0
    return fm + ld + t_int + t_flt + to_i + ult_res + max_i + min_i
end
