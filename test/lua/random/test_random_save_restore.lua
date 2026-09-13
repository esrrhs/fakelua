package "RandomTest"

function test_random_save_restore()
    -- Simple test: generate value, save state, generate another value,
    -- restore state, generate again - should match
    local rng = random.new(42)

    local v1 = rng:int(1, 1000000)
    local state = rng:get_state()
    local v2 = rng:int(1, 1000000)

    -- Restore and regenerate
    rng:set_state(state)
    local v3 = rng:int(1, 1000000)

    if v2 ~= v3 then return 0 end

    -- Also test float save/restore
    local rng2 = random.new(123)
    local f1 = rng2:float(0.0, 1.0)
    local state2 = rng2:get_state()
    local f2 = rng2:float(0.0, 1.0)
    rng2:set_state(state2)
    local f3 = rng2:float(0.0, 1.0)
    if f2 ~= f3 then return 0 end

    return 1
end
