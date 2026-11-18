function chunked_http_server(typ, connid, req)
    assert(typ == "request")
    if req.path == "/request" then
        assert(req.body == "abcde" and req.query == "mode=chunk")
        return {status = 201, headers = {["Transfer-Encoding"] = "ChUnKeD"}, body = "3;name=value \t\r\nabc\r\n2\r\nde\r\n0\r\nX-Trailer: done\r\n\r\n"}, 99
    end
    if req.path == "/empty" then
        return {headers = {["Transfer-Encoding"] = "chunked"}, body = "0\r\n\r\n"}
    end
    if req.path == "/badhex" then
        return {headers = {["Transfer-Encoding"] = "chunked"}, body = "nothex\r\na\r\n0\r\n\r\n"}
    end
    if req.path == "/empty-size" then
        return {headers = {["Transfer-Encoding"] = "chunked"}, body = "\r\n"}
    end
    if req.path == "/bad-end" then
        return {headers = {["Transfer-Encoding"] = "chunked"}, body = "1\r\naXX0\r\n\r\n"}
    end
    if req.path == "/too-large" then
        return {headers = {["Transfer-Encoding"] = "chunked"}, body = "ffffffffffffffff\r\n"}
    end
    if req.path == "/partial-size" then
        return {headers = {["Transfer-Encoding"] = "chunked"}, body = "a"}
    end
    if req.path == "/partial-body" then
        return {headers = {["Transfer-Encoding"] = "chunked"}, body = "5\r\nab"}
    end
    if req.path == "/partial-trailer" then
        return {headers = {["Transfer-Encoding"] = "chunked"}, body = "0\r\nX-Trailer: unfinished\r\n"}
    end
    if req.path == "/oversized-header" then
        return {headers = {["X-Large"] = string.rep("h", 300000)}, body = ""}
    end
    local status = tonumber(string.sub(req.path, 2))
    return {status = status, headers = {["Server"] = "custom"}, body = ""}
end

function chunked_http_done(req, err, response)
    req.done = true
    req.error = err
    if response ~= nil then
        req.result_body = response.body
        req.result_status = response.status
    end
end

local function await_chunked_request(req, label)
    for i = 1, 3000 do
        runtime.tick()
        if req.done then break end
        os.sleep(1)
    end
    local done = req.done
    req:close()
    assert(done, "HTTP completion timeout: " .. tostring(label))
end

function test_http_chunked_framing()
    -- Malformed/incomplete chunk streams finish when the peer closes. Use the
    -- server's bounded idle timeout rather than waiting for its 10-second default.
    local server = http.server({ip = "127.0.0.1", port = 0, timeout_ms = 250})
    server:dispatch("chunked_http_server")
    local base = "http://127.0.0.1:" .. tostring(server:get_port())
    local request = http.request({method = "POST", url = base .. "/request?mode=chunk", version = 10, headers = {["Transfer-Encoding"] = "chunked"}, body = "3\r\nabc\r\n2\r\nde\r\n0\r\n\r\n"}, "chunked_http_done")
    await_chunked_request(request, "chunked request")
    assert(request.error == nil and request.result_status == 201 and request.result_body == "abcde")
    request = http.get(base .. "/empty", "chunked_http_done")
    await_chunked_request(request, "empty chunk")
    assert(request.error == nil and request.result_body == "")
    local invalid = {"badhex", "empty-size", "bad-end", "too-large", "partial-size", "partial-body", "partial-trailer", "oversized-header"}
    for i = 1, #invalid do
        request = http.get(base .. "/" .. invalid[i], "chunked_http_done")
        await_chunked_request(request, invalid[i])
        assert(type(request.error) == "string" and #request.error > 0, invalid[i])
    end
    request = http.request({url = base .. "/empty", headers = {["X-Large"] = string.rep("h", 300000)}}, "chunked_http_done")
    await_chunked_request(request, "oversized request header")
    assert(type(request.error) == "string")
    local statuses = {204, 301, 302, 400, 404, 500}
    for i = 1, #statuses do
        request = http.get(base .. "/" .. tostring(statuses[i]), "chunked_http_done")
        await_chunked_request(request, tostring(statuses[i]))
        assert(request.error == nil and request.result_status == statuses[i])
    end
    server:close()
    return 1
end

function test_http_configuration_contracts()
    assert(not pcall(function() return http.request(1) end))
    assert(not pcall(function() return http.server(1) end))
    assert(not pcall(function() return http.server({port = 0, tls = true}) end))
    assert(not pcall(function() return http.server({port = 0, ip = "not-an-address"}) end))
    local server = http.server({port = 0, tls = 0, backlog = 8})
    server:close()
    server = http.server({port = 0, tls = "disable"})
    server:close()
    local urls = {"http://[", "ftp://localhost/file", "http:/missing-host"}
    for i = 1, #urls do
        local request = http.request({url = urls[i], tls_verify = "false", timeout_ms = 100}, "chunked_http_done")
        await_chunked_request(request, urls[i])
        assert(type(request.error) == "string" and #request.error > 0)
    end
    return 1
end

function deferred_http_handler(typ, connid, request)
    assert(typ == "request" and request.path == "/deferred")
    remember_http_connection(connid)
    return nil
end

function test_http_deferred_reply()
    assert(not pcall(function() http.request(false) end))
    local server = http.server({ip = "127.0.0.1", port = 0})
    server:dispatch("deferred_http_handler")
    local request = http.get("http://127.0.0.1:" .. tostring(server:get_port()) .. "/deferred", "chunked_http_done")
    local connid = 0
    for i = 1, 5000 do
        runtime.tick()
        connid = last_http_connection()
        if connid > 0 then break end
        os.sleep(1)
    end
    assert(connid > 0)
    assert(not server:reply(-1, "missing connection"))
    local ok, message = pcall(function() server:reply(connid, {reason = "bad\r\nreason"}) end)
    assert(not ok and string.find(message, "reason", 1, true))
    ok, message = pcall(function() server:reply(connid, {headers = {invalid = "bad\r\nheader"}}) end)
    assert(not ok and string.find(message, "header", 1, true))
    assert(server:reply(connid, {status = 202, reason = "Deferred", headers = {["X-Test"] = "retained"}, body = "deferred body"}))
    await_chunked_request(request, "deferred reply")
    assert(request.error == nil and request.result_status == 202 and request.result_body == "deferred body")
    server:close()
    assert(not server:reply(connid, "already closed"))
    return 1
end
