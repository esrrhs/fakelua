package "NetCreate"

function test_server_create_destroy()
    local srv = net.server({ port = 19999, maxconn = 10 })
    srv:close()
    return 1
end

function test_client_create_destroy()
    local c = net.client({ port = 19998 })
    c:close()
    return 1
end

function test_server_stop_restart()
    local srv = net.server({ port = 19997, maxconn = 4 })
    srv:close()
    local srv2 = net.server({ port = 19997, maxconn = 4 })
    srv2:close()
    return 1
end

function test_ws_path_crlf()
    local ok = pcall(function()
        net.client({ port = 19996, framer = "websocket", ws_path = "/x\r\nHost: evil" })
    end)
    if ok then return 0 end
    return 1
end
