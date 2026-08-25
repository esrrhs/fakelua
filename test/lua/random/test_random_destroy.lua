package "RandomTest"

function test_random_destroy()
    -- Create and immediately test, no crash
    for i = 1, 10 do
        local rng = random.new(i)
        rng:int(1, 100)
        rng:float(0.0, 1.0)
        rng:dice(2, 6)
        rng:chance(0.5)
        rng:weighted({1, 2, 3})
        rng:get_state()
    end

    -- Create many RNGs to test group creation
    local rngs = {}
    for i = 1, 50 do
        rngs[i] = random.new(i * 1000)
    end

    -- Use them all
    for i = 1, 50 do
        rngs[i]:int(1, 100)
    end

    return 1
end
