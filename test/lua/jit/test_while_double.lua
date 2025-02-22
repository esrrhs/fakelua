function test(a, b)
    local i = 0
    local result = 0

    while i < a do
        local j = 0

        while j < b do
            result = result + (i * j)
            j = j + 1
        end

        i = i + 1
    end

    return result
end
