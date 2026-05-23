-- Specializable function with a bare 'return' (no expression).
-- Exercises type_inferencer.cpp line 847: CollectReturnExps pushes nullptr
-- when the return statement has no expression.
-- test(5) = 10, test(-1) = nil (from bare return).
function test(n)
    if n <= 0 then
        return
    end
    return n * 2
end
