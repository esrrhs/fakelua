function test_os_clock()
    -- os.clock() returns a float >= 0
    local c1 = os.clock()
    if type(c1) ~= "number" then return 0 end
    if c1 < 0 then return 0 end

    -- do some work
    local sum = 0
    for i = 1, 1000 do
        sum = sum + i
    end

    local c2 = os.clock()
    if type(c2) ~= "number" then return 0 end
    -- c2 should be >= c1
    if c2 < c1 then return 0 end

    return 6000
end
