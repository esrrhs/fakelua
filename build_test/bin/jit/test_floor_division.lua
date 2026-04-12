-- Test floor division semantics (Bug #2)
-- Lua // is floor division (rounds toward negative infinity)

function test_pos(a, b)
    return a // b
end

function test_neg(a, b)
    return a // b
end

function test_both_neg(a, b)
    return a // b
end

function test_pos_div_neg(a, b)
    return a // b
end