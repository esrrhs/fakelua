package "TimerTest"

function on_timer(type, data)
end

function test_del_before_fire()
    local id = timer.set(5000, "TimerTest.on_timer")

    local ok = timer.del(id)
    if ok ~= true then return 0 end

    -- 再删一次应返回 false
    ok = timer.del(id)
    if ok ~= false then return 0 end

    return 1
end
