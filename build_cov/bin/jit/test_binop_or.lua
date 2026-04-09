function test(a, b, c, d)
    local e = 2 + a or b
    local f = c or d - 1
    return e, f
end
