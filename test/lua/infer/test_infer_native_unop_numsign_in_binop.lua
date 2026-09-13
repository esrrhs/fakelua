-- NUMBER_SIGN unop in InferArgTypeForSpec: exercises line 1401.
-- In the expression #s + 0, InferArgTypeForSpec is called on the #s operand
-- (unop NUMBER_SIGN). NUMBER_SIGN always returns T_INT.
-- Both operands T_INT -> native fast path for PLUS is taken.
-- #"hello" + 0 = 5.
function test()
    local s = "hello"
    return #s + 0
end
