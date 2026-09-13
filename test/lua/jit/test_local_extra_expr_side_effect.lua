-- Test that extra expressions in local declarations are evaluated.
-- In Lua, `local a, b = expr1, expr2, expr3` should evaluate all three
-- expressions, even though the result of expr3 is discarded.

local function make_counter()
    local obj = new_global_obj("counter", "counter")
    timer.register_obj_methods(obj)
    obj:set_int("count", 0)
    return obj
end

local function side_effect(obj)
    obj:add_int("count", 1)
    return 99
end

-- Test: extra expression is evaluated (side effect happens)
function test()
    local obj = make_counter()
    local a = 1, side_effect(obj)
    return a, obj:get_int("count")
end

-- Test: multiple extra function calls are all evaluated
function test_func_call()
    local obj = make_counter()
    local a = 1, side_effect(obj), side_effect(obj)
    return a, obj:get_int("count")
end

-- Test: extra expression values are discarded (only first assigned)
function test_values_discarded()
    local function get_three()
        return 10, 20, 30
    end
    local a, b = get_three(), 99
    return a, b
end
