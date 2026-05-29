function test()
    local t = {}
    t[0] = 42
    if t[0] ~= 42 then
        return 1
    end
    t[0] = nil
    if t[0] ~= nil then
        return 2
    end
    t[0] = 100
    return t
end
