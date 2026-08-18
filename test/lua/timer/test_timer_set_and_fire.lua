package "TimerTest"

-- 回调：通过 timer.result 记录调用
function on_timer(type, data)
    -- type == "timer", data == timer id
    timer.result("count", timer.result("count") + 1)
    timer.result("last_id", data)
end

function test_set_and_fire()
    timer.result("count", 0)
    local id = timer.set(1, "TimerTest.on_timer")
    if id == nil then return 0 end

    -- 等待定时器到期
    local now = os.clock()
    while os.clock() - now < 0.5 do
        timer.tick()
        if timer.result("count") > 0 then
            break
        end
    end

    -- 验证回调确实被调用了一次
    if timer.result("count") ~= 1 then return 0 end
    -- 验证回调收到正确的 timer id
    if timer.result("last_id") ~= id then return 0 end

    return 1
end
