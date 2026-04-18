function test()
    local t = { a = 1, b = 2, c = 3 }
    t["a"] = nil
    if t["a"] == nil then
        return t["c"]
    end
    return -1
end
