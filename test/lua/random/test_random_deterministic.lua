package "RandomTest"

function test_random_deterministic()
    -- Two RNGs with same seed must produce identical sequences
    local rng1 = random.new(42)
    local rng2 = random.new(42)

    for i = 1, 100 do
        if rng1:int(1, 1000000) ~= rng2:int(1, 1000000) then
            return 0
        end
    end

    -- After consuming some values, states must still match
    for i = 1, 50 do
        rng1:float(0.0, 1.0)
        rng2:float(0.0, 1.0)
    end

    local s1 = rng1:get_state()
    local s2 = rng2:get_state()
    if type(s1) ~= "string" then return 0 end
    if s1 ~= s2 then return 0 end

    -- Different seeds should produce different sequences
    local rng3 = random.new(43)
    local rng4 = random.new(44)
    local all_same = true
    for i = 1, 10 do
        if rng3:int(1, 1000000) ~= rng4:int(1, 1000000) then
            all_same = false
            break
        end
    end
    if all_same then return 0 end

    return 1
end
