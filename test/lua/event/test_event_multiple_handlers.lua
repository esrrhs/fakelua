package "EventTest"

function handler_a(data)
    local o = get_global_obj("event_result")
    if o then
        o:add_int("order", 10)
        o:set_int("a_data", data)
    end
end

function handler_b(data)
    local o = get_global_obj("event_result")
    if o then
        o:add_int("order", 1)
        o:set_int("b_data", data)
    end
end

function handler_c(data)
    local o = get_global_obj("event_result")
    if o then
        o:add_int("order", 1)
        o:set_int("c_data", data)
    end
end

function test_multiple_handlers()
    local obj = new_global_obj("event_result", "event_result")
    timer.register_obj_methods(obj)
    obj:set_int("order", 0)
    obj:set_int("a_data", 0)
    obj:set_int("b_data", 0)
    obj:set_int("c_data", 0)

    event.on("custom", "EventTest.handler_a")
    event.on("custom", "EventTest.handler_b")
    event.on("custom", "EventTest.handler_c")

    event.emit("custom", 99)

    -- All three handlers should have been called
    if obj:get_int("a_data") ~= 99 then return 0 end
    if obj:get_int("b_data") ~= 99 then return 0 end
    if obj:get_int("c_data") ~= 99 then return 0 end

    -- order should be 10 + 1 + 1 = 12 (all three called)
    if obj:get_int("order") ~= 12 then return 0 end

    event.clear_all()
    del_global_obj("event_result")
    return 1
end
