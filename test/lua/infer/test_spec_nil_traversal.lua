function test_nil_traversal()
    local t = { x = 10, y = 20 }
    t.x = nil
    local count = 0
    local has_x = 0
    for k, v in pairs(t) do
        count = count + 1
        if k == "x" then
            has_x = 1
        end
    end
    return count, has_x
end
