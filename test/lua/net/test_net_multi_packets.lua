package "NetMulti"

function on_server_event(type, connid, data, len, reason, state)
    if type == "conn" then
        state.conn_id = connid
    elseif type == "recv" then
        table.insert(state.recv_packets, data)
    end
end

function on_client_event(type, connid, data, len, reason, state)
    -- pass
end

function test_multi()
    local state = {}
    state.recv_packets = {}
    state.conn_id = 0

    local server = net.server({port = 19977, maxconn = 10})
    server:dispatch("NetMulti.on_server_event", state)

    local client = net.client({port = 19977})
    client:dispatch("NetMulti.on_client_event", state)

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    client:send("packet1")
    client:send("packet2")
    client:send("packet3")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    server:close()
    client:close()

    return #state.recv_packets, state.recv_packets[1], state.recv_packets[2], state.recv_packets[3]
end
