package "EventTest"

function on_unit_died(data)
    local o = get_global_obj("event_result")
    if o then
        o:add_int("count", 1)
        o:set_int("last_data", data)
    end
end

function test_on_emit()
    local obj = new_global_obj("event_result", "event_result")
    timer.register_obj_methods(obj)
    obj:set_int("count", 0)
    obj:set_int("last_data", 0)

    event.on("unit_died", "EventTest.on_unit_died")
    event.emit("unit_died", 42)

    if obj:get_int("count") ~= 1 then return 0 end
    if obj:get_int("last_data") ~= 42 then return 0 end

    event.clear_all()
    del_global_obj("event_result")
    return 1
end
