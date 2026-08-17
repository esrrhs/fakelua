package "NetTest"

-- 纯函数回调：接收事件参数，返回指令
-- 返回 "echo", data 表示让 C++ 侧将 data 发回来源连接
function on_server_event(type, connid, data, len, reason)
    if type == "recv" then
        -- 收到数据，返回 echo 指令
        return "echo", "echo:" .. data
    end
end

function on_client_event(type, connid, data, len, reason)
    -- client 只接收，不需要返回指令
end

function test_echo()
    local server = net.server({port = 19988, maxconn = 10})
    server:dispatch("NetTest.on_server_event")

    local client = net.client({port = 19988})
    client:dispatch("NetTest.on_client_event")

    -- 驱动连接建立
    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    -- client 发送
    client:send("hello fakelua")

    -- 驱动收发（server 回调会返回 echo 指令，C++ 侧执行发送）
    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    -- 从 C++ 侧读取状态
    local conn_count = server:get_conn_count()
    local recv_count = server:get_recv_count()
    local server_data = server:get_last_data()
    local client_data = client:get_last_data()

    server:close()
    client:close()

    return conn_count, recv_count, server_data, client_data
end
