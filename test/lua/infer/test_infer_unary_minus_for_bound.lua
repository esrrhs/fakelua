-- Unary minus on a T_INT variable enables the typed_int_for fast path.
-- bound = 5; for i = -bound, bound: sum = (-5)+...+5 = 0
function test()
    local bound = 5
    local sum = 0
    for i = -bound, bound do
        sum = sum + i
    end
    return sum
end
