local function test1()
    return "test", 1, 2.3
end

function test()
    local a, b, c = test1()
    return a, b, c
end
