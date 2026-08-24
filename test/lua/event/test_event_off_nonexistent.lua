package "EventTest"

function some_handler(data)
end

function test_event_off_nonexistent()
    -- Off on event with no handlers: should not crash
    event.off("nonexistent", "EventTest.some_handler")

    -- Off a handler that was never registered
    event.on("real_event", "EventTest.some_handler")
    event.off("real_event", "EventTest.some_handler")
    event.off("real_event", "EventTest.some_handler")  -- double off

    event.clear_all()
    return 1
end
