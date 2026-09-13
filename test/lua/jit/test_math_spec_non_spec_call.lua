local function f()
    return 2
end

local function my_add(x)
    return x + f()
end

local function my_add2(x)
    return x + 1.0
end

function test_call(x)
    return my_add(x)
end

function test_call_non_native(x)
    return my_add2(f())
end
