function test_negative_int_key()
    local t = { [-1] = 10, [-2] = 20, [-3] = 30 }
    local a = t[-1]
    local b = t[-2]
    local c = t[-3]
    if not (a == 10) then return 0 end
    if not (b == 20) then return 0 end
    if not (c == 30) then return 0 end
    return 1
end

function test_negative_int_key_write()
    local t = { [-1] = 10, [-2] = 20 }
    t[-1] = 50
    if not (t[-1] == 50) then return 0 end
    if not (t[-2] == 20) then return 0 end
    return 1
end

function test_negative_int_key_mixed()
    local t = { [-1] = 10, [1] = 20, x = 30 }
    if not (t[-1] == 10) then return 0 end
    if not (t[1] == 20) then return 0 end
    if not (t.x == 30) then return 0 end
    return 1
end
