package "NetTest"

function on_server_event(type, connid, data, len, reason, state)
    if type == "conn" then
        state.server_connid = connid
        state.conn_count = state.conn_count + 1
    elseif type == "recv" then
        state.server_data = data
        state.recv_count = state.recv_count + 1
        state.server:send(connid, "echo:" .. data)
    end
end

function on_client_event(type, connid, data, len, reason, state)
    if type == "recv" then
        state.client_data = data
    end
end

function test_echo()
    local state = {}
    state.server_data = ""
    state.server_connid = 0
    state.client_data = ""
    state.conn_count = 0
    state.recv_count = 0

    local server = net.server({port = 19988, maxconn = 10})
    state.server = server
    server:dispatch("NetTest.on_server_event", state)

    local client = net.client({port = 19988})
    client:dispatch("NetTest.on_client_event", state)

    for i = 1, 50 do
        server:tick()
        client:tick()
    end

    client:send("hello fakelua")

    for i = 1, 50 do
        server:tick()
        client:tick()
    end

    server:close()
    client:close()

    -- 调试：打印到 stderr
    print("DEBUG: conn=" .. state.conn_count .. " recv=" .. state.recv_count .. " data=[" .. state.server_data .. "]")

    return state.conn_count, state.recv_count, state.server_data, state.client_data
end
