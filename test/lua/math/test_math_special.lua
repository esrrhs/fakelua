function test_math_special()
    if math.abs(math.erf(0)) > 1e-12 then return 0 end
    if math.abs(math.erfc(0) - 1) > 1e-12 then return 0 end
    if math.abs(math.erf(1) + math.erfc(1) - 1) > 1e-10 then return 0 end
    if math.abs(math.gamma(1) - 1) > 1e-12 then return 0 end
    if math.abs(math.gamma(5) - 24) > 1e-9 then return 0 end
    if math.abs(math.lgamma(1)) > 1e-12 then return 0 end
    if math.abs(math.lgamma(5) - math.log(24)) > 1e-9 then return 0 end
    return 1
end
