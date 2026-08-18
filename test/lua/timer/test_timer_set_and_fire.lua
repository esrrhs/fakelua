package "TimerTest"

function on_timer(type, data)
end

function test_set_and_fire()
    local id = timer.set(1, "TimerTest.on_timer")
    if id == nil then return 0 end

    -- 等待定时器到期
    local now = os.clock()
    while os.clock() - now < 0.5 do
        timer.tick()
    end

    return 1
end
