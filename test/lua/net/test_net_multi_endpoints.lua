package "NetMultiEndpoints"

-- 每个 server/client 独立的回调函数
function on_server_a_event(type, connid, data, len, reason)
    if type == "recv" then
        return "echo", "from_a:" .. data
    end
end

function on_server_b_event(type, connid, data, len, reason)
    if type == "recv" then
        return "echo", "from_b:" .. data
    end
end

function on_client_event(type, connid, data, len, reason)
    -- client 只接收，不需要返回指令
end

-- 测试：2个 server + 2个 client，各自独立通信，互不干扰
function test_multi_servers_multi_clients()
    local port_a = 19971
    local port_b = 19972

    local server_a = net.server({port = port_a, maxconn = 10})
    server_a:dispatch("NetMultiEndpoints.on_server_a_event")

    local server_b = net.server({port = port_b, maxconn = 10})
    server_b:dispatch("NetMultiEndpoints.on_server_b_event")

    local client_a = net.client({port = port_a})
    client_a:dispatch("NetMultiEndpoints.on_client_event")

    local client_b = net.client({port = port_b})
    client_b:dispatch("NetMultiEndpoints.on_client_event")

    -- 驱动连接建立
    for i = 1, 50 do
        server_a:tick()
        server_b:tick()
        client_a:tick()
        client_b:tick()
    end

    -- 各 client 向各自 server 发送数据
    client_a:send("hello_a")
    client_b:send("hello_b")

    -- 驱动收发
    for i = 1, 50 do
        server_a:tick()
        server_b:tick()
        client_a:tick()
        client_b:tick()
    end

    local conn_a = server_a:get_conn_count()
    local recv_a = server_a:get_recv_count()
    local data_a = server_a:get_last_data()
    local echo_a = client_a:get_last_data()

    local conn_b = server_b:get_conn_count()
    local recv_b = server_b:get_recv_count()
    local data_b = server_b:get_last_data()
    local echo_b = client_b:get_last_data()

    server_a:close()
    server_b:close()
    client_a:close()
    client_b:close()

    return conn_a, recv_a, data_a, echo_a, conn_b, recv_b, data_b, echo_b
end


-- 测试：1个 server + 多个 client 同时连接，server 能分别处理每个连接
function test_one_server_multi_clients()
    local port = 19973
    local server = net.server({port = port, maxconn = 10})
    server:dispatch("NetMultiEndpoints.on_server_a_event")

    local client1 = net.client({port = port})
    client1:dispatch("NetMultiEndpoints.on_client_event")

    local client2 = net.client({port = port})
    client2:dispatch("NetMultiEndpoints.on_client_event")

    local client3 = net.client({port = port})
    client3:dispatch("NetMultiEndpoints.on_client_event")

    -- 驱动连接建立
    for i = 1, 60 do
        server:tick()
        client1:tick()
        client2:tick()
        client3:tick()
    end

    -- 3个 client 各自发送
    client1:send("msg1")
    client2:send("msg2")
    client3:send("msg3")

    -- 驱动收发
    for i = 1, 60 do
        server:tick()
        client1:tick()
        client2:tick()
        client3:tick()
    end

    local conn_count = server:get_conn_count()
    local recv_count = server:get_recv_count()

    -- 每个 client 都应该收到对应的 echo
    local echo1 = client1:get_last_data()
    local echo2 = client2:get_last_data()
    local echo3 = client3:get_last_data()

    server:close()
    client1:close()
    client2:close()
    client3:close()

    return conn_count, recv_count, echo1, echo2, echo3
end

