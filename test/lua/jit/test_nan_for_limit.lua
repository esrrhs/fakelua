function test_nan_for_limit()
    local n = 0
    for i = 1, 0 / 0, -1 do
        n = n + 1
        if n > 2 then return n end
    end
    return n
end
