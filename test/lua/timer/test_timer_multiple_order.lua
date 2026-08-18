package "TimerTest"

local recorder = nil

function on_timer(type, data)
    if recorder then
        recorder:increment(data)
    end
end

function test_multiple_timers_order()
    recorder = timer.create_recorder()

    -- 设置 3 个定时器：先设大延迟，再设小延迟
    local id1 = timer.set(200, "TimerTest.on_timer")
    local id2 = timer.set(50, "TimerTest.on_timer")
    local id3 = timer.set(120, "TimerTest.on_timer")

    -- 等待全部触发
    local now = os.clock()
    while os.clock() - now < 1.0 do
        timer.tick()
        if recorder:get_count() >= 3 then
            break
        end
    end

    -- 验证 3 个回调都被调用
    if recorder:get_count() ~= 3 then return 0 end
    if recorder:get_order_count() ~= 3 then return 0 end

    -- 验证触发顺序：id2(50ms) → id3(120ms) → id1(200ms)
    if recorder:get_order_at(1) ~= id2 then return 0 end
    if recorder:get_order_at(2) ~= id3 then return 0 end
    if recorder:get_order_at(3) ~= id1 then return 0 end

    recorder = nil
    return 1
end
