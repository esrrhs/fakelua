-- Test modulo semantics (Bug #3)
-- Lua % follows floor division semantics: a % b = a - b * floor(a / b)

function test(a, b)
    return a % b
end