package "EventTest"

function once_handler(data)
    local o = get_global_obj("event_result")
    if o then
        o:add_int("once_count", 1)
        o:set_int("once_data", data)
    end
end

function test_event_once()
    local obj = new_global_obj("event_result", "event_result")
    timer.register_obj_methods(obj)
    obj:set_int("once_count", 0)
    obj:set_int("once_data", 0)

    event.once("one_shot", "EventTest.once_handler")

    -- First emit: handler should fire
    event.emit("one_shot", 100)
    if obj:get_int("once_count") ~= 1 then return 0 end
    if obj:get_int("once_data") ~= 100 then return 0 end

    -- Second emit: handler should NOT fire (auto-removed)
    event.emit("one_shot", 200)
    if obj:get_int("once_count") ~= 1 then return 0 end
    if obj:get_int("once_data") ~= 100 then return 0 end

    event.clear_all()
    del_global_obj("event_result")
    return 1
end
