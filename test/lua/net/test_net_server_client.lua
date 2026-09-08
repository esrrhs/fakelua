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
    for i = 1, 50 do
        runtime.tick()
        if server:get_conn_count() >= 1 then break end
        os.sleep(1)
    end

    -- client 发送
    client:send("hello fakelua")

    -- 驱动收发（server 回调会返回 echo 指令，C++ 侧执行发送）
    for i = 1, 50 do
        runtime.tick()
        if #client:get_last_data() > 0 then break end
        os.sleep(1)
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

function on_close_in_recv(type, connid, data, len, reason)
    if type == "recv" then
        return "close"
    end
end

function test_close_in_recv()
    local server = net.server({port = 19991, maxconn = 4})
    server:dispatch("NetTest.on_close_in_recv")
    local client = net.client({port = 19991})
    for i = 1, 50 do
        runtime.tick()
        if server:get_conn_count() >= 1 then break end
        os.sleep(1)
    end
    client:send("bye")
    for i = 1, 50 do
        runtime.tick()
        if server:get_conn_count() == 0 then break end
        os.sleep(1)
    end
    client:close()
    -- close 已在 recv 回调里延后执行，再 close 是 no-op
    server:close()
    return 1
end

function test_slot_reuse_repeated_connect()
    local srv = net.server({port = 19985, maxconn = 2})
    srv:dispatch("NetTest.on_server_event")

    local success_count = 0
    for iter = 1, 6 do
        local client = net.client({port = 19985})
        client:dispatch("NetTest.on_client_event")
        local ok = false
        for i = 1, 50 do
            runtime.tick()
            if srv:get_conn_count() >= 1 then
                ok = client:send("ping_" .. iter)
                if ok then break end
            end
            os.sleep(1)
        end
        if ok then
            for i = 1, 50 do
                runtime.tick()
                if client:get_last_data() == "echo:ping_" .. iter then
                    success_count = success_count + 1
                    break
                end
                os.sleep(1)
            end
        end
        client:close()
        for i = 1, 20 do
            runtime.tick()
            if srv:get_conn_count() == 0 then break end
            os.sleep(1)
        end
    end
    srv:close()
    return success_count
end

function test_client_connect_fail()
    local client = net.client({port = 19921})
    client:dispatch("NetTest.on_client_event")
    for i = 1, 50 do
        runtime.tick()
        os.sleep(1)
    end
    local sent = client:send("should fail")
    client:close()
    return sent and 0 or 1
end

function test_send_buffer_full()
    local srv = net.server({port = 19986, maxconn = 2, sendbuf = 64})
    local cli = net.client({port = 19986})
    for i = 1, 50 do
        runtime.tick()
        if srv:get_conn_count() >= 1 then break end
        os.sleep(1)
    end
    local big = string.rep("A", 1024)
    local ok = srv:send(srv:get_connid(), big)
    srv:close()
    cli:close()
    return ok and 0 or 1
end

