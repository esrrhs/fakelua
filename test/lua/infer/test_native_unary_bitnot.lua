-- Native bitwise NOT fast path: when operand is T_INT, CompileUnop
-- generates ~(n) directly instead of the OpBitNot macro.
-- test(0) == -1 (all bits flipped), test(1) == -2.
function test(n)
    return ~n + n * 2
end
