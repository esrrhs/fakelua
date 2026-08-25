package "RandomTest"

function test_random_basic()
    local rng = random.new(42)

    -- Test int range [1, 10]
    for i = 1, 100 do
        local n = rng:int(1, 10)
        if n < 1 or n > 10 then return 0 end
        if n ~= math.floor(n) then return 0 end
    end

    -- Test int range [-50, 50]
    for i = 1, 100 do
        local n = rng:int(-50, 50)
        if n < -50 or n > 50 then return 0 end
    end

    -- Test float range [0.0, 1.0)
    for i = 1, 100 do
        local f = rng:float(0.0, 1.0)
        if f < 0.0 or f >= 1.0 then return 0 end
    end

    -- Test float range [-10.0, 10.0)
    for i = 1, 100 do
        local f = rng:float(-10.0, 10.0)
        if f < -10.0 or f >= 10.0 then return 0 end
    end

    -- Test single value range
    for i = 1, 10 do
        local n = rng:int(5, 5)
        if n ~= 5 then return 0 end
    end

    -- Wide signed range: lo+offset must not overflow int64
    local hi = 9223372036854775807
    for i = 1, 50 do
        local n = rng:int(-1, hi)
        if n < -1 or n > hi then return 0 end
    end

    return 1
end
