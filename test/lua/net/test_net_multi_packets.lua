package "NetMulti"

-- 纯函数回调：server 收到数据返回 echo 指令
function on_server_event(type, connid, data, len, reason)
    if type == "recv" then
        return "echo", data -- 原样发回
    end
end

function on_client_event(type, connid, data, len, reason)
    -- 只接收
end

function test_multi()
    local server = net.server({ port = 19977, maxconn = 10 })
    server:dispatch("NetMulti.on_server_event")

    local client = net.client({ port = 19977 })
    client:dispatch("NetMulti.on_client_event")

    for i = 1, 50 do
        runtime.tick()
        if server:get_conn_count() >= 1 then break end
        os.sleep(1)
    end

    -- 发送多个包
    client:send("packet1")
    client:send("packet2")
    client:send("packet3")

    for i = 1, 50 do
        runtime.tick()
        if server:get_recv_count() >= 3 and client:get_last_data() == "echo:packet3" then break end
        os.sleep(1)
    end

    -- 从 C++ 侧读取状态
    local recv_count = server:get_recv_count()
    local last_data = client:get_last_data()

    server:close()
    client:close()

    return recv_count, last_data
end
