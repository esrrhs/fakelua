function inner(a, b)
    return a >= b
end

function test(a, b)
    local c = "inner"
    return c(a, b)
end
