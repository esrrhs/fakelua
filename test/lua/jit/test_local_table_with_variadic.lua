function test(...)
    local t = {
        ...
    }
    return t
end

function run_test()
    local t = test(10, 20)
    return t[1] + t[2]
end
