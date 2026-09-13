function test(a, b)
    local i = 1
    local result = 0
    repeat
        local j = 1
        repeat
            result = result + (i * j)
            if result > 5 then
                break
            end
            j = j + 1
        until j >= b
        i = i + 1
    until i >= a
    return result
end
