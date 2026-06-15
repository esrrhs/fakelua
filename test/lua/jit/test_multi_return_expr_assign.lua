local function get_multi()
    return 10, 20, 30
end

function test()
    local a, b = get_multi(), 100
    return a, b
end
