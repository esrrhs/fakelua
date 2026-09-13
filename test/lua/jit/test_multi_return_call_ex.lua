local function test1()
    return 1, 2.3
end

local function test2(a, b, c)
    return a, b, c
end

function test()
    return test2("test", test1(), 2.4)
end
