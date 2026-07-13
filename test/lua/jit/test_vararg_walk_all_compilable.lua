function helper(x)
    return x + 100
end
function test(...)
    -- FunctionCall + Args
    local a = helper(5)
    -- While
    local i = 0
    while i < 2 do
        i = i + 1
    end
    -- Repeat
    repeat
        i = i + 1
    until i > 3
    -- ElseIfList
    local j = 0
    if i == 0 then
        j = 1
    elseif i == 1 then
        j = 2
    end
    -- ForIn
    local s = 0
    for k, v in pairs({10, 20}) do
        s = s + k
    end
    return a + i + j + s
end
