function test(a, b)
    while a < 10 do
        a = a + 1
        b = b .. "2"
    end
    return a, b
end
