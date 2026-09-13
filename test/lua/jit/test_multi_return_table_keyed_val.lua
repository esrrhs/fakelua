local function get_multi()
    return 3, 4
end

function test()
    local t = { a = get_multi() }
    return t
end
