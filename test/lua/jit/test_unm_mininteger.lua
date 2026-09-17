function test_unm_mininteger()
    local t = {math.mininteger}
    local v = -t[1]
    if math.type(v) ~= "integer" then return 0 end
    if v ~= math.mininteger then return 0 end
    return 1
end
