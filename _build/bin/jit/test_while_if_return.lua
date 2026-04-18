function test(a, b)
    while a < b  do
        if a > 5 then
            a = a + 1
        else
            return a
        end
    end
    return b
end
