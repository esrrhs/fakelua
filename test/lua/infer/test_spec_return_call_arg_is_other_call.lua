-- func2(n) = n * 2 and func1(n) = n + 1 are both math-param functions.
-- caller returns func1(func2(n)): func2's result is used as the argument to func1.
-- The return-type inferencer must resolve func2(n) -> T_INT and then use that
-- as the argument type for func1, yielding a native int64_t return for caller.
-- caller(5) == 11, caller(3) == 7.
function func2(n)
    return n * 2
end

function func1(n)
    return n + 1
end

function caller(n)
    return func1(func2(n))
end
