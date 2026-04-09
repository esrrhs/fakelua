function test(a, b)
    local i = 0
    local result = 0
    repeat
        local j = 0
        repeat
            result = result + (i * j)
            j = j + 1
        until j >= b
        i = i + 1
    until i >= a
    return result
end
