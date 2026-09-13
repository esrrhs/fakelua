-- InferPrefixExp "exp" branch: a parenthesised expression  (a + 2).
-- In the Lua grammar  (expr)  is a PrefixExp of type "exp", wrapped inside
-- an Exp of type "prefixexp".  InferPrefixExp handles this by delegating to
-- InferNode on the inner expression.  Here (a + 2) = T_INT + T_INT = T_INT,
-- so b is specialised as int64_t and the typed addition path is taken.
-- Expected return value: 5.
function test()
    local a = 3
    local b = (a + 2)
    return b
end
