function swap(a, b)
    return b, a
end

function three()
    return 10, 20, 30
end

function add_pair(a, b)
    return a + b
end

function test_basic()
    local x, y = swap(3, 7)
    return x + y
end

function test_three()
    local a, b, c = three()
    return a + b + c
end

function test_forward()
    local a, b, c = three()
    return a, b, c
end

function main()
    local r1 = test_basic()
    local r2 = test_three()
    return r1 + r2
end