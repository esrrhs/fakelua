package "TimerTest"

-- 心跳回调里再 timer.tick()：必须先推进下一跳，否则会无限递归
function on_heartbeat_nested(type, data)
    local o = get_global_obj("timer_result")
    if o then
        o:add_int("hb_count", 1)
        timer.tick()
    end
end

function test_heartbeat_nested()
    local obj = new_global_obj("timer_result", "timer_result")
    timer.register_obj_methods(obj)

    obj:set_int("hb_count", 0)
    timer.set_heartbeat(50, "TimerTest.on_heartbeat_nested")

    local now = os.clock()
    while os.clock() - now < 0.15 do
        timer.tick()
        if obj:get_int("hb_count") >= 1 then
            break
        end
    end

    local count = obj:get_int("hb_count")
    if count < 1 then return 0 end

    del_global_obj("timer_result")
    return 1
end
