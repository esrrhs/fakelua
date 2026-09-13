function test()
    local funcs = {}
    for i = 1, 5 do
        funcs[i] = function()
            return i * 10
        end
    end
    return funcs[1]() + funcs[2]() + funcs[3]() + funcs[4]() + funcs[5]()
end
