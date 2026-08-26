package "NetWsTest"

function on_server_event(type, connid, data, len, reason)
    if type == "recv" then
        return "echo", "echo:" .. data
    end
end

function on_client_event(type, connid, data, len, reason)
end

function test_ws_echo()
    local server = net.ws_server({port = 19992, maxconn = 10, ws_path = "/"})
    server:dispatch("NetWsTest.on_server_event")

    local client = net.ws_client({port = 19992, ws_path = "/"})
    client:dispatch("NetWsTest.on_client_event")

    for i = 1, 50 do
        server:tick()
        client:tick()
    end

    client:send("hello websocket")

    for i = 1, 50 do
        server:tick()
        client:tick()
    end

    local conn_count = server:get_conn_count()
    local recv_count = server:get_recv_count()
    local server_data = server:get_last_data()
    local client_data = client:get_last_data()

    server:close()
    client:close()

    return conn_count, recv_count, server_data, client_data
end
