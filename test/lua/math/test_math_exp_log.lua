function test_math_exp_log()
    local e1 = math.exp(1.0)
    local l1 = math.log(e1)
    local l2 = math.log(100.0, 10.0)
    local l10 = math.log10(100.0)
    local s1 = math.asin(0.0)
    local c1 = math.acos(1.0)
    local t1 = math.atan(0.0)
    local t2 = math.atan(1.0, 1.0)
    local sh = math.sinh(0.0)
    local ch = math.cosh(0.0)
    local th = math.tanh(0.0)
    return l1 + l2 + l10 + s1 + c1 + t1 + sh + (ch - 1.0) + th
end
