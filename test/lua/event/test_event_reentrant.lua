package "EventTest"

function reentrant_handler(data)
    local o = get_global_obj("event_result")
    if o then
        o:add_int("count", 1)
    end

    -- Re-entrant: emit another event during handler execution
    if data == 1 then
        event.emit("second_event", 2)
    end
end

function second_handler(data)
    local o = get_global_obj("event_result")
    if o then
        o:set_int("second_data", data)
    end
end

function test_event_reentrant()
    local obj = new_global_obj("event_result", "event_result")
    timer.register_obj_methods(obj)
    obj:set_int("count", 0)
    obj:set_int("second_data", 0)

    event.on("first_event", "EventTest.reentrant_handler")
    event.on("second_event", "EventTest.second_handler")

    -- Emit first_event, which will re-entrantly emit second_event
    event.emit("first_event", 1)

    -- Both handlers should have been called
    if obj:get_int("count") ~= 1 then return 0 end
    if obj:get_int("second_data") ~= 2 then return 0 end

    event.clear_all()
    del_global_obj("event_result")
    return 1
end
