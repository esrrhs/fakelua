function test(a, b, c, d)
    local e = a and (b + 1) or (c + 1)
    local f = d and (b + 1) or (c + 1)
    return e, f
end
