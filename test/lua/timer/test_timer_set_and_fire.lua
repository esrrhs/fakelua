package "TimerTest"

local recorder = nil

function on_timer(type, data)
    -- type == "timer", data == timer id
    if recorder then
        recorder:increment(data)
    end
end

function test_set_and_fire()
    recorder = timer.create_recorder()

    local id = timer.set(1, "TimerTest.on_timer")
    if id == nil then return 0 end

    -- 等待定时器到期
    local now = os.clock()
    while os.clock() - now < 0.5 do
        timer.tick()
        if recorder:get_count() > 0 then
            break
        end
    end

    -- 验证回调确实被调用了一次
    if recorder:get_count() ~= 1 then return 0 end
    -- 验证回调收到正确的 timer id
    if recorder:get_order_at(1) ~= id then return 0 end

    recorder = nil
    return 1
end
