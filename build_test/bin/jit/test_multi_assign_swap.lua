-- Test multi-assignment swap semantics (Bug #1)
-- Lua requires evaluating all RHS expressions first, then assigning

function test_swap(a, b)
    a, b = b, a
    return a
end

function test_swap_b(a, b)
    a, b = b, a
    return b
end