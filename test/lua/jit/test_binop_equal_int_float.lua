-- Tests that integer and float with equal mathematical value compare as equal (Lua semantics).
function test_int_eq_float(a, b)
    local r = a == b
    return r
end

function test_float_eq_int(a, b)
    local r = a == b
    return r
end

function test_int_neq_float(a, b)
    local r = a == b
    return r
end
