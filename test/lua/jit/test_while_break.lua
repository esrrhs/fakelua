function test(a, b)
    local i = 1
    local result = 0
    while i < a do
        local j = 1
        while j < b do
            result = result + (i * j)
            if result > 5 then
                break
            end
            j = j + 1
        end
        i = i + 1
    end
    return result
end
