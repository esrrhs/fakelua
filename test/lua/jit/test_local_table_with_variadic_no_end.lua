function test(a, b, ...)
    local t = {
        ...,
        a,
        b
    }
    return t
end

function run_test()
    local t = test(10, 20, 30, 40)
    return t[1] + t[2] + t[3]
end
