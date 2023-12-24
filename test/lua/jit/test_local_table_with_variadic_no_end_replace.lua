function test(a, b, ...)
    local t = {
        [1] = a,
        ...,
        b
    }
    return t
end
