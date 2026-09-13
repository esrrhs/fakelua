local function my_sin_nonspec(x)
    return x + 0.0
end

local function my_non_specialized_func(s)
    return my_sin_nonspec(#s)
end

function test_non_spec_unsupported(s)
    return my_non_specialized_func(s)
end
