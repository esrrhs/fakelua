-- Dynamic for-loop with step=0 (via helper to force CVar path) must throw "'for' step is zero"
local function zero_step()
    return 0
end

function test()
    local sum = 0
    for i = 1, 10, zero_step() do
        sum = sum + i
    end
    return sum
end
