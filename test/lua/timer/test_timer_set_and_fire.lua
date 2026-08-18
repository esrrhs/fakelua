package "TimerTest"

-- 回调：通过 timer.set_result 记录调用
function on_timer(type, data)
    -- type == "timer", data == timer id
    local count = timer.get_result("count")
    timer.set_result("count", count + 1)
    timer.set_result("last_id", data)
end

function test_set_and_fire()
    timer.set_result("count", 0)
    local id = timer.set(1, "TimerTest.on_timer")
    if id == nil then return 0 end

    -- 等待定时器到期
    local now = os.clock()
    while os.clock() - now < 0.5 do
        timer.tick()
        if timer.get_result("count") > 0 then
            break
        end
    end

    -- 验证回调确实被调用了一次
    if timer.get_result("count") ~= 1 then return 0 end
    -- 验证回调收到正确的 timer id
    if timer.get_result("last_id") ~= id then return 0 end

    return 1
end
