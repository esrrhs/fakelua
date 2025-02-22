function test(a, b)
    repeat
        a = a + 1
        b = b .. "2"
    until a >= 3
    return a, b
end
