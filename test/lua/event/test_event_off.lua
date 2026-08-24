package "EventTest"

function handler_should_not_run(data)
    local o = get_global_obj("event_result")
    if o then
        o:add_int("count", 1)
    end
end

function test_event_off()
    local obj = new_global_obj("event_result", "event_result")
    timer.register_obj_methods(obj)
    obj:set_int("count", 0)

    event.on("test_off", "EventTest.handler_should_not_run")
    event.off("test_off", "EventTest.handler_should_not_run")

    event.emit("test_off", 1)

    -- Handler should NOT have been called
    if obj:get_int("count") ~= 0 then return 0 end

    -- Emit again to be sure
    event.emit("test_off", 2)
    if obj:get_int("count") ~= 0 then return 0 end

    event.clear_all()
    del_global_obj("event_result")
    return 1
end
