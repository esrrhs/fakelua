package "TimerTest"

function on_timer(type, data)
    local count = timer.result("count")
    timer.result("count", count + 1)
    -- 记录第 count+1 次触发的 timer id
    timer.result("id_" .. tostring(count + 1), data)
end

function test_multiple_timers_order()
    timer.result("count", 0)
    local id1 = timer.set(200, "TimerTest.on_timer")
    local id2 = timer.set(50, "TimerTest.on_timer")
    local id3 = timer.set(120, "TimerTest.on_timer")

    -- 等待全部触发
    local now = os.clock()
    while os.clock() - now < 1.0 do
        timer.tick()
        if timer.result("count") >= 3 then
            break
        end
    end

    -- 验证 3 个回调都被调用
    if timer.result("count") ~= 3 then return 0 end

    -- 验证触发顺序：id2(50ms) → id3(120ms) → id1(200ms)
    if timer.result("id_1") ~= id2 then return 0 end
    if timer.result("id_2") ~= id3 then return 0 end
    if timer.result("id_3") ~= id1 then return 0 end

    return 1
end
