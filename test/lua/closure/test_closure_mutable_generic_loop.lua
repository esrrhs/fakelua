local function nested_forward_goto(flag)
    local trace = 0
    do
        if flag then goto outside end
        trace = trace + 1
    end
    ::outside::
    return trace
end
function test_mutable_generic_loop()
    local captured = {}
    for key, value in ipairs({2, 4, 6}) do
        value = value + 1
        captured[key] = function() return value end
        if key == 1 then continue end
        if key == 2 then break end
    end
    assert(captured[1]() == 3 and captured[2]() == 5 and captured[3] == nil)
    assert(nested_forward_goto(true) == 0 and nested_forward_goto(false) == 1)
    return 1
end


local function custom_iteration(state, previous)
    local index = previous + 1
    if index <= #state then return index, state[index] end
    return nil
end

local function declaration_pair() return 3, 4 end

function test_generic_iterator_capture_and_multivalue_prefix_declarations()
    local captured = {}
    for index, value in custom_iteration, {1, 2, 3}, 0 do
        captured[index] = function() return value end
        value = value + 10
        if index == 1 then continue end
        if index == 2 then break end
    end
    assert(captured[1]() == 11 and captured[2]() == 12 and captured[3] == nil)
    local prefix, a, b = 7, declaration_pair()
    local get_prefix = function() return prefix end
    prefix = prefix + 1
    assert(get_prefix() == 8 and a == 3 and b == 4)
    local shadowed = 10
    do
        local shadowed, c, d = 20, declaration_pair()
        assert(shadowed == 20 and c == 3 and d == 4)
    end
    assert(shadowed == 10)
    local text, e, f = tostring(12), declaration_pair()
    assert(text == "12" and e == 3 and f == 4)
    return 1
end
