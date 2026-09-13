local function get_multi()
    return 3, 4
end

function test()
    local t = { [get_multi()] = "hello" }
    return t
end
