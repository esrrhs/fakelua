package "EventTest"

function handler_2args(a, b)
    local o = get_global_obj("event_result")
    if o then
        o:set_int("arg_a", a)
        o:set_int("arg_b", b)
    end
end

function handler_4args(a, b, c, d)
    local o = get_global_obj("event_result")
    if o then
        o:set_int("arg4_a", a)
        o:set_int("arg4_b", b)
        o:set_int("arg4_c", c)
        o:set_int("arg4_d", d)
    end
end

function test_args_forward()
    local obj = new_global_obj("event_result", "event_result")
    timer.register_obj_methods(obj)
    obj:set_int("arg_a", 0)
    obj:set_int("arg_b", 0)
    obj:set_int("arg4_a", 0)
    obj:set_int("arg4_b", 0)
    obj:set_int("arg4_c", 0)
    obj:set_int("arg4_d", 0)

    -- Test 2 args
    event.on("args2", "EventTest.handler_2args")
    event.emit("args2", 11, 22)
    if obj:get_int("arg_a") ~= 11 then return 0 end
    if obj:get_int("arg_b") ~= 22 then return 0 end

    -- Test 4 args
    event.on("args4", "EventTest.handler_4args")
    event.emit("args4", 1, 2, 3, 4)
    if obj:get_int("arg4_a") ~= 1 then return 0 end
    if obj:get_int("arg4_b") ~= 2 then return 0 end
    if obj:get_int("arg4_c") ~= 3 then return 0 end
    if obj:get_int("arg4_d") ~= 4 then return 0 end

    event.clear_all()
    del_global_obj("event_result")
    return 1
end
