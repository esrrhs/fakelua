-- func2 has direct arithmetic n+1, so it is specialised with a math param.
-- func1 only calls func2(n), no direct arithmetic.  After the nested-call fix,
-- the TypeInferencer detects that func1's parameter n reaches func2's math
-- param position and therefore specialises func1 too.
-- test calls func1(n) — same reasoning applies one level higher.
-- All three functions must produce int64_t specialisations and return native
-- int64_t (not CVar) when called with an integer argument.
-- test(10) == 11, test(2.5) == 3.5.
function func2(n)
    return n + 1
end

function func1(n)
    return func2(n)
end

function test(n)
    return func1(n)
end
