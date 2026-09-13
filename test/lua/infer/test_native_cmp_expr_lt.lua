-- Native comparison < in expression context: both operands numeric,
-- CompileBinop generates SET_BOOL with native < operator.
-- test(n) returns n*2 < 10 ? 1 : 0.
-- test(3) -> 6<10 -> 1, test(7) -> 14<10 -> 0.
function test(n)
    local x = n * 2
    if x < 10 then
        return 1
    end
    return 0
end
