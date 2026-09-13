local function get_multi()
    return 3, 4
end

function test()
    local t = { 1, 2, get_multi(), 5 }
    return t
end
