package "EventTest"

function handler_x(data)
    local o = get_global_obj("event_result")
    if o then o:add_int("x_count", 1) end
end

function handler_y(data)
    local o = get_global_obj("event_result")
    if o then o:add_int("y_count", 1) end
end

function test_event_clear_all()
    local obj = new_global_obj("event_result", "event_result")
    timer.register_obj_methods(obj)
    obj:set_int("x_count", 0)
    obj:set_int("y_count", 0)

    event.on("evt_x", "EventTest.handler_x")
    event.once("evt_y", "EventTest.handler_y")

    -- Fire both
    event.emit("evt_x", 1)
    event.emit("evt_y", 1)
    if obj:get_int("x_count") ~= 1 then return 0 end
    if obj:get_int("y_count") ~= 1 then return 0 end

    -- Clear everything
    event.clear_all()

    -- Neither should fire
    event.emit("evt_x", 2)
    event.emit("evt_y", 2)
    if obj:get_int("x_count") ~= 1 then return 0 end
    if obj:get_int("y_count") ~= 1 then return 0 end

    del_global_obj("event_result")
    return 1
end
