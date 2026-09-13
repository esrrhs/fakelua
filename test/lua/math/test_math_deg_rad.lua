function test_math_deg_rad()
    local deg180 = math.deg(math.pi)
    local rad_pi = math.rad(180.0)
    if not (math.abs(deg180 - 180.0) < 1e-4) then return 0 end
    if not (math.abs(rad_pi - math.pi) < 1e-4) then return 0 end
    return 200
end
