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
