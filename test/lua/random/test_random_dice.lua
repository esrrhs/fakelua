package "RandomTest"

function test_random_dice()
    local rng = random.new(123)

    -- 1d6: range [1, 6]
    for i = 1, 100 do
        local roll = rng:dice(1, 6)
        if roll < 1 or roll > 6 then return 0 end
    end

    -- 2d6: range [2, 12]
    for i = 1, 100 do
        local roll = rng:dice(2, 6)
        if roll < 2 or roll > 12 then return 0 end
    end

    -- 3d20: range [3, 60]
    for i = 1, 100 do
        local roll = rng:dice(3, 20)
        if roll < 3 or roll > 60 then return 0 end
    end

    -- 1d1: always 1
    for i = 1, 10 do
        local roll = rng:dice(1, 1)
        if roll ~= 1 then return 0 end
    end

    return 1
end
