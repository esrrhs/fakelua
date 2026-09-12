package "NetWssTest"

function on_server_event(type, connid, data, len, reason)
    if type == "recv" then
        return "echo", "echo:" .. data
    end
end

function on_client_event(type, connid, data, len, reason)
end

function test_wss_echo()
    local server = net.ws_server({
        port = 19993,
        maxconn = 10,
        ws_path = "/",
        tls = true,
        cert = "./tls/cert.pem",
        key = "./tls/key.pem"
    })
    server:dispatch("NetWssTest.on_server_event")

    local client = net.ws_client({
        port = 19993,
        ws_path = "/",
        tls = true,
        tls_verify = false
    })
    client:dispatch("NetWssTest.on_client_event")

    for i = 1, 80 do
        runtime.tick()
        if server:get_conn_count() >= 1 then break end
        os.sleep(1)
    end

    client:send("hello wss")

    for i = 1, 80 do
        runtime.tick()
        if #client:get_last_data() > 0 then break end
        os.sleep(1)
    end

    local conn_count = server:get_conn_count()
    local recv_count = server:get_recv_count()
    local server_data = server:get_last_data()
    local client_data = client:get_last_data()

    server:close()
    client:close()

    return conn_count, recv_count, server_data, client_data
end
