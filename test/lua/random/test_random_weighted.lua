package "RandomTest"

function test_random_weighted()
    local rng = random.new(777)

    -- weights {1, 0, 0} should always pick index 1
    for i = 1, 50 do
        local idx = rng:weighted({1, 0, 0})
        if idx ~= 1 then return 0 end
    end

    -- weights {0, 0, 1} should always pick index 3
    for i = 1, 50 do
        local idx = rng:weighted({0, 0, 1})
        if idx ~= 3 then return 0 end
    end

    -- weights {1, 1}: should pick index 1 or 2
    for i = 1, 100 do
        local idx = rng:weighted({1, 1})
        if idx ~= 1 and idx ~= 2 then return 0 end
    end

    -- weights {1, 1, 1, 1}: should pick 1-4
    for i = 1, 100 do
        local idx = rng:weighted({1, 1, 1, 1})
        if idx < 1 or idx > 4 then return 0 end
    end

    -- Empty table should return nil
    local idx = rng:weighted({})
    if idx ~= nil then return 0 end

    return 1
end
