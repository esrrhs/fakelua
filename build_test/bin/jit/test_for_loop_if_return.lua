function test(a, b)
    local result = 0
    for i = a, b do
        if i > 5 then
            result = i
        else
            return i
        end
    end
    return result
end
