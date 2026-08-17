package "NetCreate"

function test_server_create_destroy()
    local srv = net.server({port = 19999, maxconn = 10})
    srv:close()
    return 1
end

function test_client_create_destroy()
    local c = net.client({port = 19998})
    c:close()
    return 1
end
