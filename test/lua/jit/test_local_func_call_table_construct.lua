local function inner(t)
    return t[1] >= t[2]
end

function test(a, b)
    return inner { a, b }
end
