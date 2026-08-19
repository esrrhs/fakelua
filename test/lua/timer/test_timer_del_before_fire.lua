package "TimerTest"

-- 回调：记录调用
function on_timer(type, data)
    local o = get_global_obj("timer_result")
    if o then
        o:set_int("count", o:get_int("count") + 1)
    end
end

function test_del_before_fire()
    -- 创建全局 NativeObject（无需 group_id，直接通过 string key 索引）
    local obj = new_global_obj("timer_result", "timer_result")
    timer.register_obj_methods(obj)

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

    del_global_obj("timer_result")
    return 1
end
