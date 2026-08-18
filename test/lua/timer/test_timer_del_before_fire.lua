package "TimerTest"

-- 回调：记录调用
function on_timer(type, data)
    local o = get_native_obj("timer_result", 1)
    if o then
        o:set_int("count", o:get_int("count") + 1)
    end
end

function test_del_before_fire()
    -- 在函数内创建 NativeObject
    local gid = new_native_group()
    local obj = new_native_obj(gid, "timer_result", 1)
    timer.register_obj_methods("timer_result", 1)

    obj:set_int("count", 0)
    local id = timer.set(5000, "TimerTest.on_timer")

    local ok = timer.del(id)
    if ok ~= true then return 0 end

    -- 等待足够时间，确认定时器不会触发
    local now = os.clock()
    while os.clock() - now < 0.3 do
        timer.tick()
    end

    -- 删除后回调不应被调用
    if obj:get_int("count") ~= 0 then return 0 end

    -- 再删一次应返回 false
    ok = timer.del(id)
    if ok ~= false then return 0 end

    del_native_group(gid)
    return 1
end
