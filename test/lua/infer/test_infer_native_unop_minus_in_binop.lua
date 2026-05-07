-- Native MINUS unop in InferArgTypeForSpec: exercises line 1392.
-- In the expression -x + y, InferArgTypeForSpec is called on the -x operand
-- (unop MINUS), which recursively calls InferArgTypeForSpec on x (T_INT).
-- Both operands return T_INT -> native fast path for PLUS is taken.
-- -3 + 7 = 4.
function test()
    local x = 3
    local y = 7
    return -x + y
end
