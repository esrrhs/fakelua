function test(...)
    local a, b = ..., 1
    return b, a
end

function run_test()
    return test(10, 20)
end

function run_empty_test()
    return test()
end
