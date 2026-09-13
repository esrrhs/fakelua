local function my_sin_arg(x)
    return x + 0.0
end

local function my_sin(x, s)
    return x + my_sin_arg(#s)
end

function test_sin(x, s)
    return my_sin(x, s)
end
