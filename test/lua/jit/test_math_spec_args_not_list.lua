local function my_sin(x)
    return x + 0.0
end

local function my_sin_caller(x)
    if x > 100 then
        return x + my_sin "abc"
    end
    return x
end

function test_args_not_list(x)
    return my_sin_caller(x)
end
