package "TimerTest"

function on_timer(type, data)
end

function test_multiple_timers_order()
    -- 设置 3 个定时器：先设大延迟，再设小延迟
    local id1 = timer.set(200, "TimerTest.on_timer")
    local id2 = timer.set(50, "TimerTest.on_timer")
    local id3 = timer.set(120, "TimerTest.on_timer")

    -- 等待全部触发
    local now = os.clock()
    while os.clock() - now < 1.0 do
        timer.tick()
    end

    -- 全部触发后，这些 id 已删除，再删应返回 false
    if timer.del(id1) ~= false then return 0 end
    if timer.del(id2) ~= false then return 0 end
    if timer.del(id3) ~= false then return 0 end

    return 1
end
