package "TimerTest"

-- 回调：通过 get_native_obj 获取对象并记录调用
function on_timer(type, data)
    local o = get_native_obj("timer_result", 1)
    if o then
        o:set_int("count", o:get_int("count") + 1)
        o:set_int("last_id", data)
    end
end

function test_set_and_fire()
    -- 在函数内创建 NativeObject
    local gid = new_native_group()
    local obj = new_native_obj(gid, "timer_result", 1)
    timer.register_obj_methods("timer_result", 1)

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

    del_native_group(gid)
    return 1
end
