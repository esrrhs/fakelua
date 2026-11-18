function test_native_udp_callbacks()
    local server = net.udp_server({ip = "127.0.0.1", port = 0})
    local client = net.udp_server({ip = "127.0.0.1", port = 0})
    server:dispatch("cpp_udp_echo")
    client:dispatch("cpp_udp_receive")
    local payload = "a\0b"
    assert(client:send(payload, "127.0.0.1", server:get_port()))
    for i = 1, 3000 do
        runtime.tick()
        if client:get_last_data() == payload then break end
        os.sleep(1)
    end
    local received = client:get_last_data()
    local server_event = server:get_events()
    local client_event = client:get_events()
    server:close()
    client:close()
    assert(received == payload and server_event == "recv" and client_event == "recv")
    assert(server:get_events() == nil and client:get_events() == nil)
    return 1
end
