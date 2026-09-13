function test_neg_int_spec()
    local t = { [-1] = 10, [-2] = 20, [-3] = 30 }
    local a = t[-1]
    local b = t[-2]
    local c = t[-3]
    if not (a == 10) then return 0 end
    if not (b == 20) then return 0 end
    if not (c == 30) then return 0 end
    t[-1] = 50
    if not (t[-1] == 50) then return 0 end
    return 1
end
