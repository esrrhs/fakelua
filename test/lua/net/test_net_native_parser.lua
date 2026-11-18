function native_parser_server_event(kind, connid, data, length, reason)
    if kind == "recv" then
        native_parser_host_check(data)
        return "echo", data
    end
    return nil
end

local function wait_native_parser(count)
    for i = 1, 5000 do
        runtime.tick()
        if native_parser_received_count() >= count then break end
        os.sleep(1)
    end
    assert(native_parser_received_count() == count)
end

function test_native_parser_and_dispatch_recovery()
    local server = net.server({ip = "127.0.0.1", port = 19971, parser = "native_three_byte_parser"})
    server:dispatch("native_parser_server_event")
    local client = net.client({ip = "127.0.0.1", port = 19971, parser = "native_three_byte_parser"})
    client:dispatch("native_parser_client_event")
    for i = 1, 100 do runtime.tick() os.sleep(1) end
    assert(client:send("abcdef"))
    wait_native_parser(2)
    assert(client:send("err"))
    local failed = false
    for i = 1, 5000 do
        local ok, message = pcall(function() runtime.tick() end)
        if not ok then
            assert(string.find(message, "network dispatch failure", 1, true))
            failed = true
            break
        end
        os.sleep(1)
    end
    assert(failed)
    assert(client:send("ghi"))
    wait_native_parser(3)
    assert(server:close_connection(0))
    runtime.tick()
    assert(not server:close_connection(0))
    client:close()
    server:close()
    return 1
end


function contract_parser_default(data) return data, nil end
function contract_parser_clamped(data) return data, #data + 100 end
function contract_parser_wait(data) return nil end
function contract_parser_empty(data) return "", nil end

function custom_contract_server_event(kind, connid, data, length, reason)
    if kind == "recv" then return "echo", data end
    return nil
end

function custom_contract_client_event(kind, connid, data, length, reason)
    if kind == "recv" then
        custom_contract_seen(data)
        return "close", nil
    end
    return nil
end

function test_custom_parser_return_contracts()
    local parsers = {"contract_parser_default", "contract_parser_clamped", "contract_parser_wait", "contract_parser_empty", "missing_contract_parser"}
    for i = 1, #parsers do
        local port = 19980 + i
        local server = net.server({ip = "127.0.0.1", port = port, framer = "custom", parser = parsers[i]})
        server:dispatch(string.rep("custom", 1) .. "_contract_server_event")
        local client = net.client({ip = "127.0.0.1", port = port, framer = "raw"})
        client:dispatch("custom_contract_client_event")
        for j = 1, 100 do
            runtime.tick()
            if server:get_conn_count() > 0 then break end
            os.sleep(1)
        end
        assert(server:get_conn_count() == 1)
        assert(not pcall(function() server:send(0) end))
        assert(server:send(0, false))
        assert(client:send(false))
        assert(client:send("abc"))
        if i <= 2 then
            for j = 1, 1000 do
                runtime.tick()
                if custom_contract_received() >= i then break end
                os.sleep(1)
            end
            assert(custom_contract_received() == i)
            assert(server:get_recv_count() == 1)
        else
            for j = 1, 10 do runtime.tick() os.sleep(1) end
            assert(server:get_recv_count() == 0)
        end
        client:close()
        server:close()
    end
    local udp = net.udp_server({ip = "127.0.0.1", port = 19988})
    assert(not pcall(function() udp:send("x", "127.0.0.1", 0) end))
    udp:close()
    assert(not pcall(function() net.udp_client(false) end))
    return 1
end
