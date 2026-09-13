package "NetUdpTest"

function on_server(type, connid, data, len, reason)
    if type == "recv" then
        return "echo", "echo:" .. data
    end
end

function on_client(type, connid, data, len, reason)
end

function test_udp_echo()
    local server = net.udp_server({ ip = "127.0.0.1", port = 0 })
    server:dispatch("NetUdpTest.on_server")
    local port = server:get_port()
    if port < 1 or port > 65535 then
        server:close()
        return 0
    end

    local client = net.udp_client({ ip = "127.0.0.1", port = port })
    client:dispatch("NetUdpTest.on_client")
    client:send("hello udp")

    for i = 1, 80 do
        runtime.tick()
        if client:get_last_data() == "echo:hello udp" then break end
        os.sleep(1)
    end

    local server_data = server:get_last_data()
    local client_data = client:get_last_data()
    local recv_count = server:get_recv_count()
    local peer_ip, peer_port = server:get_peer()

    server:close()
    client:close()

    if server_data ~= "hello udp" then return 0 end
    if client_data ~= "echo:hello udp" then return 0 end
    if recv_count < 1 then return 0 end
    if peer_ip ~= "127.0.0.1" then return 0 end
    if peer_port < 1 then return 0 end
    return 1
end
