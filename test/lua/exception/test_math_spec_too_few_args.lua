local function my_add(x, y)
    return x + y
end

function test_fewer_args(x)
    local z = my_add(x) + 1.0
    return z
end
