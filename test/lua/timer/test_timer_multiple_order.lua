package "TimerTest"

-- 回调：记录调用顺序
function on_timer(type, data)
    local o = get_native_obj("timer_result", 1)
    if o then
        local count = o:get_int("count")
        o:set_int("count", count + 1)
        o:set_int("id_" .. tostring(count + 1), data)
    end
end

function test_multiple_timers_order()
    -- 在函数内创建 NativeObject
    local gid = new_native_group()
    local obj = new_native_obj(gid, "timer_result", 1)
    timer.register_obj_methods("timer_result", 1)

    obj:set_int("count", 0)
    local id1 = timer.set(200, "TimerTest.on_timer")
    local id2 = timer.set(50, "TimerTest.on_timer")
    local id3 = timer.set(120, "TimerTest.on_timer")

    -- 等待全部触发
    local now = os.clock()
    while os.clock() - now < 1.0 do
        timer.tick()
        if obj:get_int("count") >= 3 then
            break
        end
    end

    -- 验证 3 个回调都被调用
    if obj:get_int("count") ~= 3 then return 0 end

    -- 验证触发顺序：id2(50ms) → id3(120ms) → id1(200ms)
    if obj:get_int("id_1") ~= id2 then return 0 end
    if obj:get_int("id_2") ~= id3 then return 0 end
    if obj:get_int("id_3") ~= id1 then return 0 end

    del_native_group(gid)
    return 1
end
