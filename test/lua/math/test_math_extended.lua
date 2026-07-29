function test_math_constants()
    local pi_val = math.pi
    local huge_val = math.huge
    local max_int = math.maxinteger
    local min_int = math.mininteger

    if not (pi_val > 3.14 and pi_val < 3.15) then return 0 end
    if not (huge_val > 1e300) then return 0 end
    if not (max_int > 9000000000000000000) then return 0 end
    if not (min_int < -9000000000000000000) then return 0 end
    return 100
end

function test_math_deg_rad()
    local deg180 = math.deg(math.pi)
    local rad_pi = math.rad(180.0)
    if not (math.abs(deg180 - 180.0) < 1e-4) then return 0 end
    if not (math.abs(rad_pi - math.pi) < 1e-4) then return 0 end
    return 200
end

function test_math_random()
    math.randomseed(12345)
    local r1 = math.random()
    if not (r1 >= 0.0 and r1 < 1.0) then return 0 end

    local r2 = math.random(10)
    if not (r2 >= 1 and r2 <= 10) then return 0 end

    local r3 = math.random(50, 100)
    if not (r3 >= 50 and r3 <= 100) then return 0 end
    return 300
end

function test_math_modf_frexp()
    local i, f = math.modf(3.14)
    if not (i == 3.0 and math.abs(f - 0.14) < 1e-4) then return 0 end

    local frac, exp = math.frexp(8.0)
    if not (frac == 0.5 and exp == 4) then return 0 end
    return 400
end
