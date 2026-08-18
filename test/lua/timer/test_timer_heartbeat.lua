package "TimerTest"

function on_heartbeat(type, data)
end

function test_heartbeat()
    timer.set_heartbeat(20, "TimerTest.on_heartbeat")

    -- 等待足够时间让心跳触发多次
    local now = os.clock()
    while os.clock() - now < 0.2 do
        timer.tick()
    end

    return 1
end
