package "TimerTest"

-- 回调：通过 get_global_obj 获取全局对象并记录调用
function on_timer(type, data)
    local o = get_global_obj("timer_result")
    if o then
        o:set_int("count", o:get_int("count") + 1)
        o:set_int("last_id", data)
    end
end

function test_set_and_fire()
    -- 创建全局 NativeObject（无需 group_id，直接通过 string key 索引）
    local obj = new_global_obj("timer_result", "timer_result")
    timer.register_obj_methods(obj)

    obj:set_int("count", 0)
    local id = timer.set(1, "TimerTest.on_timer")
    if id == nil then return 0 end

    -- 等待定时器到期
    local now = os.clock()
    while os.clock() - now < 0.5 do
        timer.tick()
        if obj:get_int("count") > 0 then
            break
        end
    end

    -- 验证回调确实被调用了一次
    if obj:get_int("count") ~= 1 then return 0 end
    -- 验证回调收到正确的 timer id
    if obj:get_int("last_id") ~= id then return 0 end

    del_global_obj("timer_result")
    return 1
end
