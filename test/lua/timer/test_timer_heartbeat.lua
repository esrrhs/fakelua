package "TimerTest"

function on_heartbeat(type, data)
    timer.result("hb_count", timer.result("hb_count") + 1)
end

function test_heartbeat()
    timer.result("hb_count", 0)
    timer.set_heartbeat(20, "TimerTest.on_heartbeat")

    -- 等待足够时间让心跳触发多次
    local now = os.clock()
    while os.clock() - now < 0.2 do
        timer.tick()
    end

    -- 心跳 20ms 间隔，200ms 内应触发多次（预期约 10 次，允许一定调度误差）
    local count = timer.result("hb_count")
    if count < 3 then return 0 end

    return 1
end
