function test(a, b)
    while a < 3 do
        a = a + 1
        b = b .. "2"
    end
    return a
end

function test2(a, b)
    while a < 3 do
        a = a + 1
        b = b .. "2"
    end
    return b
end
