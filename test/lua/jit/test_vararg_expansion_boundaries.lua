function expansion_values() return 1, 2, 3, 4 end
function expansion_prefix(a, b, ...) return a, b, ... end
function expansion_forward(...) return expansion_prefix(...) end

function test_vararg_expansion_boundaries()
    local a, b, c, d = expansion_prefix(expansion_values())
    assert(a == 1 and b == 2 and c == 3 and d == 4)
    local e, f, g, h, i, j, k = expansion_prefix(10, 20, 30, expansion_values())
    assert(e == 10 and f == 20 and g == 30 and h == 1 and i == 2 and j == 3 and k == 4)
    a, b, c = expansion_prefix(10)
    assert(a == 10 and b == nil and c == nil)
    a, b, c, d = expansion_forward(expansion_values())
    assert(a == 1 and b == 2 and c == 3 and d == 4)
    a, b, c = expansion_prefix(expansion_values(), 9)
    assert(a == 1 and b == 9 and c == nil)
    return 1
end
