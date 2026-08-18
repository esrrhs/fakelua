package "TimerTest"

function on_timer(type, data)
    local count = timer.get_result("count")
    timer.set_result("count", count + 1)
end

function test_del_before_fire()
    timer.set_result("count", 0)
    local id = timer.set(5000, "TimerTest.on_timer")

    local ok = timer.del(id)
    if ok ~= true then return 0 end

    -- 等待足够时间，确认定时器不会触发
    local now = os.clock()
    while os.clock() - now < 0.3 do
        timer.tick()
    end

    -- 删除后回调不应被调用
    if timer.get_result("count") ~= 0 then return 0 end

    -- 再删一次应返回 false
    ok = timer.del(id)
    if ok ~= false then return 0 end

    return 1
end
