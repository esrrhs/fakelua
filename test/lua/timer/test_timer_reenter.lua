package "TimerTest"

-- 回调里再 timer.set：tick 必须先从 map 摘掉再派发，否则 rehash 后 UB
function on_reenter(type, data)
    local o = get_global_obj("timer_result")
    if o then
        o:add_int("count", 1)
        if o:get_int("count") == 1 then
            timer.set(1, "TimerTest.on_reenter")
        end
    end
end

function test_reenter()
    local obj = new_global_obj("timer_result", "timer_result")
    timer.register_obj_methods(obj)

    obj:set_int("count", 0)
    local id = timer.set(1, "TimerTest.on_reenter")
    if id == nil then return 0 end

    local now = os.clock()
    while os.clock() - now < 0.5 do
        timer.tick()
        if obj:get_int("count") >= 2 then
            break
        end
    end

    if obj:get_int("count") ~= 2 then return 0 end
    del_global_obj("timer_result")
    return 1
end

-- 回调里 timer.tick() 必须 no-op，delay-0 定时器不能在同一次 tick 里再爆一层
function on_nested_tick(type, data)
    local o = get_global_obj("timer_result")
    if o then
        o:add_int("count", 1)
        timer.tick()
        timer.set(0, "TimerTest.on_nested_tick")
    end
end

function test_nested_tick_noop()
    local obj = new_global_obj("timer_result", "timer_result")
    timer.register_obj_methods(obj)
    obj:set_int("count", 0)
    local id = timer.set(0, "TimerTest.on_nested_tick")
    if id == nil then return 0 end
    timer.tick()
    if obj:get_int("count") ~= 1 then return 0 end
    timer.tick()
    if obj:get_int("count") ~= 2 then return 0 end
    del_global_obj("timer_result")
    return 1
end
