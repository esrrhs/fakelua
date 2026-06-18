function test(...)
    local a, b = ...
    return b, a
end

function run_test()
    return test(10, 20)
end
