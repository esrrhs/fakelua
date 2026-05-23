-- T_DYNAMIC unary minus: exercises OpUnaryMinus in CompileBinop CVar path
-- (c_gen.cpp line 2497).
-- t[1] is a T_DYNAMIC table lookup; -t[1] uses OpUnaryMinus.
-- -(10) = -10.
function test(a)
    local t = {a}
    local x = t[1]
    return -x
end
