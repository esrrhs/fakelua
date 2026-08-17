package "NetTest"

-- 简单的 echo server + client 测试
-- 验证: conn 回调触发、recv 收到数据、send 发送成功、close 触发

function test_echo()
    -- 创建 server
    local server = net.server({port = 19888, maxconn = 10})

    -- 记录事件
    local events = {}

    -- 注册回调
    server:dispatch("on_server_event")

    -- 创建 client
    local client = net.client({port = 19888})
    client:dispatch("on_client_event")

    -- 驱动几帧让连接建立
    for i = 1, 10 do
        server:tick()
        client:tick()
    end

    -- client 发送消息
    client:send("hello fakelua")

    -- 驱动几帧让数据到达
    for i = 1, 10 do
        server:tick()
        client:tick()
    end

    -- server 回声
    if last_server_data then
        server:send(last_server_connid, "echo:" .. last_server_data)
    end

    -- 驱动几帧让回声到达
    for i = 1, 10 do
        server:tick()
        client:tick()
    end

    -- 清理
    client:close()
    server:close()

    return events
end

-- server 回调
function on_server_event(type, connid, data, len, reason)
    if type == "conn" then
        last_server_connid = connid
    elseif type == "recv" then
        last_server_data = data
    end
    table.insert(events, type)
end

-- client 回调
function on_client_event(type, connid, data, len, reason)
    if type == "recv" then
        last_client_data = data
    end
    table.insert(events, "c_" .. type)
end

return test_echo
