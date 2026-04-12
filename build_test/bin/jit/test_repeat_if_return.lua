function test(a, b)
    repeat
        if a > 5 then
            a = a+ 1
        else
            return a
        end
    until a > b
    return b
end
