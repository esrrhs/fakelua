package "EventTest"

function test_event_no_handlers()
    -- Emit on event with no handlers: should not crash
    event.emit("nonexistent_event", 1, 2, 3)
    event.emit("another_nonexistent")
    return 1
end
