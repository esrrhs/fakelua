-- Table field access (kSquare var) used in an expression on the RHS.
-- Exercises InferVar line 476: InferNode(pe) is called when the var kind is
-- kSquare and GetPrefixexp() returns non-null.
-- test(5) == t[1] + 5 == 10 + 5 == 15.
function test(n)
    local t = {10, 20, 30}
    return t[1] + n
end
