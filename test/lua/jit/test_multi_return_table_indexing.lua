local function get_multi()
    return 3, 4
end

function test_get()
    local t = { [3] = "val3", [4] = "val4" }
    return t[get_multi()]
end

function test_set()
    local t = {}
    t[get_multi()] = "inserted"
    return t
end
