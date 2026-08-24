package "RandomTest"

function test_random_independent()
    -- Two RNGs with different seeds should produce different values
    local rng1 = random.new(100)
    local rng2 = random.new(200)

    local v1 = rng1:int(1, 1000000)
    local v2 = rng2:int(1, 1000000)
    if v1 == v2 then return 0 end

    -- Save rng2 state, use rng1 heavily, then verify rng2 unaffected
    local state = rng2:get_state()
    local before = rng2:int(1, 1000000)

    -- Heavy use of rng1 should not affect rng2
    for i = 1, 100 do
        rng1:int(1, 1000000)
    end

    -- Restore rng2 state and verify it produces the same next value
    rng2:set_state(state)
    local after = rng2:int(1, 1000000)
    if before ~= after then return 0 end

    return 1
end
