package "TimerTest"

-- 心跳回调：记录调用次数
function on_heartbeat(type, data)
    local o = get_native_obj("timer_result", 1)
    if o then
        o:set_int("hb_count", o:get_int("hb_count") + 1)
    end
end

function test_heartbeat()
    -- 在函数内创建 NativeObject
    local gid = new_native_group()
    local obj = new_native_obj(gid, "timer_result", 1)
    timer.register_obj_methods("timer_result", 1)

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

    del_native_group(gid)
    return 1
end
