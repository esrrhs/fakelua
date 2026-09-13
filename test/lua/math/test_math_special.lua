function test_math_special()
    if math.abs(math.erf(0)) > 1e-12 then return 0 end
    if math.abs(math.erfc(0) - 1) > 1e-12 then return 0 end
    if math.abs(math.erf(1) + math.erfc(1) - 1) > 1e-10 then return 0 end
    if math.abs(math.gamma(1) - 1) > 1e-12 then return 0 end
    if math.abs(math.gamma(5) - 24) > 1e-9 then return 0 end
    if math.abs(math.lgamma(1)) > 1e-12 then return 0 end
    if math.abs(math.lgamma(5) - math.log(24)) > 1e-9 then return 0 end
    if math.clamp(5, 1, 10) ~= 5 then return 0 end
    if math.clamp(0, 1, 10) ~= 1 then return 0 end
    if math.clamp(11, 1, 10) ~= 10 then return 0 end
    if math.type(math.clamp(5, 1, 10)) ~= "integer" then return 0 end
    if math.abs(math.clamp(5.5, 1, 10) - 5.5) > 1e-12 then return 0 end
    if math.abs(math.clamp(-1.2, 0, 1) - 0) > 1e-12 then return 0 end
    if math.abs(math.clamp(1.2, 0, 1) - 1) > 1e-12 then return 0 end
    return 1
end
