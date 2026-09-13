function make_counter(start)
    local count = start
    return function()
        count = count + 1
        return count
    end
end

function test()
    local c1 = make_counter(10)
    local c2 = make_counter(100)
    local r1 = c1()
    local r2 = c1()
    local r3 = c2()
    local r4 = c1()
    local r5 = c2()
    return r1 + r2 + r3 + r4 + r5
end
