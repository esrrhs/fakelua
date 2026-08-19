package "TimerTest"

-- 心跳回调：记录调用次数
function on_heartbeat(type, data)
    local o = get_global_obj("timer_result")
    if o then
        o:set_int("hb_count", o:get_int("hb_count") + 1)
    end
end

function test_heartbeat()
    -- 创建全局 NativeObject（无需 group_id，直接通过 string key 索引）
    local obj = new_global_obj("timer_result", "timer_result")
    timer.register_obj_methods(obj)

    obj:set_int("hb_count", 0)
    timer.set_heartbeat(20, "TimerTest.on_heartbeat")

    -- 等待足够时间让心跳触发多次
    local now = os.clock()
    while os.clock() - now < 0.2 do
        timer.tick()
    end

    -- 心跳 20ms 间隔，200ms 内应触发多次（预期约 10 次，允许一定调度误差）
    local count = obj:get_int("hb_count")
    if count < 3 then return 0 end

    del_global_obj("timer_result")
    return 1
end
