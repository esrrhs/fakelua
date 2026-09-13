package "RandomTest"

function test_random_chance()
    local rng = random.new(999)

    -- prob = 0: always false
    for i = 1, 20 do
        if rng:chance(0.0) then return 0 end
    end

    -- prob = 1: always true
    for i = 1, 20 do
        if not rng:chance(1.0) then return 0 end
    end

    -- prob = 0.5: should have both true and false in 200 trials
    local has_true = false
    local has_false = false
    for i = 1, 200 do
        if rng:chance(0.5) then
            has_true = true
        else
            has_false = true
        end
        if has_true and has_false then break end
    end
    if not has_true or not has_false then return 0 end

    -- prob = 0.3: roughly 30% true (allow 10%-50% for randomness)
    local count = 0
    for i = 1, 1000 do
        if rng:chance(0.3) then
            count = count + 1
        end
    end
    if count < 100 or count > 500 then return 0 end

    return 1
end
