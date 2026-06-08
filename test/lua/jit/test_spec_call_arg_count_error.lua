-- Calling a specialized function with fewer args than declared should be a compile error
local function my_add(x, y)
    return x + y
end

function test_fewer_args(x)
    return my_add(x)
end
