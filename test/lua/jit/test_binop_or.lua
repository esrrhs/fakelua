function test1(a, b)
    local e = 2 + a or b
    return e
end

function test2(c, d)
    local f = c or d - 1
    return f
end
