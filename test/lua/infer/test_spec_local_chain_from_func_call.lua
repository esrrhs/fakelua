-- func returns n*2 (math param, int64_t specialisation).
-- chain stores func(n) in a local, then uses it in further arithmetic.
-- Both x and y should be int64_t; the return should be native int64_t.
-- chain(5) == 5*2 + 1 = 11, chain(3) == 7.
function func(n)
    return n * 2
end

function chain(n)
    local x = func(n)
    local y = x + 1
    return y
end
