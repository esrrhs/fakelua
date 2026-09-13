-- func adds 1 to its parameter (math param).
-- wrapper stores the result in a local variable then returns it.
-- After the local-var-from-func-call fix, the local variable should be
-- declared as int64_t (not CVar) and wrapper should return int64_t natively.
-- wrapper(10) == 11, wrapper(2.5) == 3.5.
function func(n)
    return n + 1
end

function wrapper(n)
    local x = func(n)
    return x
end
