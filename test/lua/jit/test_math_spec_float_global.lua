local MY_GLOBAL = (1.5)

local function my_add(x)
    return x + MY_GLOBAL
end

function test_global(x)
    return my_add(x)
end
