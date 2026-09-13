function test(a, b)
    local result = a
    repeat
        result = b
        break
    until a >= b
    return result
end
