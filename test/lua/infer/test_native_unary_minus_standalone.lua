-- Native unary minus fast path: when operand is T_INT, CompileUnop
-- generates -(n) directly instead of the OpMinus macro.
-- test(10) == -10, test(-3) == 3.
function test(n)
    return -n + n * 2
end
