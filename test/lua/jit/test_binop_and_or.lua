function test1(a, b, c)
    local e = a and (b + 1) or (c + 1)
    return e
end

function test2(d, b, c)
    local f = d and (b + 1) or (c + 1)
    return f
end
