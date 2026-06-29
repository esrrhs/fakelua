function test_duplicate()
    local t = { x = 10, x = 20, [1] = 30, [1] = 40 }
    return t.x + t[1]
end
