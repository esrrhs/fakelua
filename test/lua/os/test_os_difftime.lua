function test_os_difftime()
    local t1 = 1000
    local t2 = 2500
    local diff = os.difftime(t2, t1)
    if diff ~= 1500 then return 0 end

    -- negative diff
    local diff2 = os.difftime(t1, t2)
    if diff2 ~= -1500 then return 0 end

    -- zero diff
    local diff3 = os.difftime(t1, t1)
    if diff3 ~= 0 then return 0 end

    return 6000
end
