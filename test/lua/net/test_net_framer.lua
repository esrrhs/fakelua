package "NetFramerTest"

function on_server_event(type, connid, data, len, reason)
    if type == "recv" then
        return "echo", "echo:" .. data
    end
end

function on_client_event(type, connid, data, len, reason)
end

-- 自定义 Lua 解包函数：以 '$' 作为包起始与结束定界符，格式形如 "$payload$"
function my_dollar_parser(buf)
    local s_idx = string.find(buf, "$", 1, true)
    if not s_idx then return nil end

    local e_idx = string.find(buf, "$", s_idx + 1, true)
    if not e_idx then return nil end

    local payload = string.sub(buf, s_idx + 1, e_idx - 1)
    local consumed = e_idx
    return payload, consumed
end

function test_framer_2be()
    local server = net.server({port = 19960, framer = "header2_be"})
    server:dispatch("NetFramerTest.on_server_event")
    local client = net.client({port = 19960, framer = "header2_be"})
    client:dispatch("NetFramerTest.on_client_event")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    client:send("hello_2be")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    local server_data = server:get_last_data()
    local client_data = client:get_last_data()

    server:close()
    client:close()

    return server_data, client_data
end

function test_framer_2le()
    local server = net.server({port = 19961, framer = "header2_le"})
    server:dispatch("NetFramerTest.on_server_event")
    local client = net.client({port = 19961, framer = "header2_le"})
    client:dispatch("NetFramerTest.on_client_event")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    client:send("hello_2le")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    local server_data = server:get_last_data()
    local client_data = client:get_last_data()

    server:close()
    client:close()

    return server_data, client_data
end

function test_framer_4le()
    local server = net.server({port = 19962, framer = "header4_le"})
    server:dispatch("NetFramerTest.on_server_event")
    local client = net.client({port = 19962, framer = "header4_le"})
    client:dispatch("NetFramerTest.on_client_event")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    client:send("hello_4le")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    local server_data = server:get_last_data()
    local client_data = client:get_last_data()

    server:close()
    client:close()

    return server_data, client_data
end

function test_framer_line()
    local server = net.server({port = 19963, framer = "line"})
    server:dispatch("NetFramerTest.on_server_event")
    local client = net.client({port = 19963, framer = "line"})
    client:dispatch("NetFramerTest.on_client_event")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    client:send("line_command_1")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    local server_data = server:get_last_data()
    local client_data = client:get_last_data()

    server:close()
    client:close()

    return server_data, client_data
end

function test_framer_fixed()
    local server = net.server({port = 19964, framer = "fixed", fixed_len = 8})
    server:dispatch("NetFramerTest.on_server_event")
    local client = net.client({port = 19964, framer = "fixed", fixed_len = 8})
    client:dispatch("NetFramerTest.on_client_event")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    client:send("12345678")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    local server_data = server:get_last_data()

    server:close()
    client:close()

    return server_data
end

function test_framer_custom_lua()
    local server = net.server({port = 19965, parser = "NetFramerTest.my_dollar_parser"})
    server:dispatch("NetFramerTest.on_server_event")
    local client = net.client({port = 19965, framer = "raw"})
    client:dispatch("NetFramerTest.on_client_event")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    -- client 以 raw 模式发送带 $ 格式的数据
    client:send("$custom_msg_dollar$")

    for i = 1, 30 do
        server:tick()
        client:tick()
    end

    local server_data = server:get_last_data()

    server:close()
    client:close()

    return server_data
end
