package "EventTest"

function handler_a(data)
    local o = get_global_obj("event_result")
    if o then o:add_int("a_count", 1) end
end

function handler_b(data)
    local o = get_global_obj("event_result")
    if o then o:add_int("b_count", 1) end
end

function test_event_clear()
    local obj = new_global_obj("event_result", "event_result")
    timer.register_obj_methods(obj)
    obj:set_int("a_count", 0)
    obj:set_int("b_count", 0)

    -- Register handlers on two different events
    event.on("evt_a", "EventTest.handler_a")
    event.on("evt_b", "EventTest.handler_b")

    -- Fire both
    event.emit("evt_a", 1)
    event.emit("evt_b", 1)
    if obj:get_int("a_count") ~= 1 then return 0 end
    if obj:get_int("b_count") ~= 1 then return 0 end

    -- Clear only evt_a
    event.clear("evt_a")

    -- evt_a handler should not fire, evt_b should still work
    event.emit("evt_a", 2)
    event.emit("evt_b", 2)
    if obj:get_int("a_count") ~= 1 then return 0 end
    if obj:get_int("b_count") ~= 2 then return 0 end

    event.clear_all()
    del_global_obj("event_result")
    return 1
end
