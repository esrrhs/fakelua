function event_contract_callback(counter)
    counter.total = counter.total + 1
end
function test_event_contracts()
    local counter = {total = 0}
    event.once("cancelled_once", "event_contract_callback")
    event.off("cancelled_once", "missing_callback")
    event.off("cancelled_once", "event_contract_callback")
    event.emit("cancelled_once", counter)
    assert(counter.total == 0)
    event.once("live_once", "event_contract_callback")
    event.emit("live_once", counter)
    event.emit("live_once", counter)
    assert(counter.total == 1)
    assert(not pcall(function() event.on("", "event_contract_callback") end))
    assert(not pcall(function() event.on("a", "") end))
    assert(not pcall(function() event.on(false, false) end))
    assert(not pcall(function() event.once("", "event_contract_callback") end))
    assert(not pcall(function() event.once("a", "") end))
    assert(not pcall(function() event.once(false, false) end))
    assert(event.emit("") == nil)
    event.clear_all()
    return 1
end
