local function get_multi()
    return 10, 20, 30
end

local function get_str_multi()
    return "hello", "world"
end

local function get_cond_bool()
    return true, false
end

function test_plus()
    local a = 5
    return a + get_multi()
end

function test_minus()
    local a = 100
    return get_multi() - a
end

function test_compare_eq()
    return get_multi() == 10
end

function test_logical_not()
    return not get_multi()
end

function test_unop_len()
    return #get_str_multi()
end

function test_concat()
    return get_str_multi() .. "_suffix"
end

function test_logical_and()
    return get_cond_bool() and "yes"
end

function test_logical_or()
    return get_cond_bool() or "no"
end
