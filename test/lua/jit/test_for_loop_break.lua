function test(a, b)
    local result = 0
    for i = a, b, 2 do
        for j = 0, i do
            result = result + j + 1
            if result > 1 then
                break
            end
        end
    end
    return result
end
