function test1()
    return 3, 4, 5
end

function test()
    return 1, 2, test1()
end
