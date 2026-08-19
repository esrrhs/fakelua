package "TimerTest"

-- 回调：记录调用顺序
function on_timer(type, data)
    local o = get_global_obj("timer_result")
    if o then
        local count = o:get_int("count")
        o:set_int("count", count + 1)
        o:set_int("id_" .. tostring(count + 1), data)
    end
end

function test_multiple_timers_order()
    -- 创建全局 NativeObject（无需 group_id，直接通过 string key 索引）
    local obj = new_global_obj("timer_result", "timer_result")
    timer.register_obj_methods(obj)

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

    del_global_obj("timer_result")
    return 1
end
