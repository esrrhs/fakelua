package "HttpTest"

function on_server(typ, connid, req)
    if typ ~= "request" then return end
    local body = "echo:" .. tostring(req.body)
    if req.method == "GET" then
        body = "hello:" .. tostring(req.path)
    end
    local resp = {}
    resp["status"] = 200
    resp["body"] = body
    return resp
end

function on_client(req, err, resp)
    req.done = true
    req.err = err
    if resp ~= nil then
        req.status = resp.status
        req.body = resp.body
    end
end

function test_echo()
    local cfg = {}
    cfg["ip"] = "127.0.0.1"
    cfg["port"] = 0
    local srv = http.server(cfg)
    srv:dispatch("HttpTest.on_server")
    local port = srv.port
    if port == nil or port == 0 then
        port = srv:get_port()
    end
    if port == nil or port == 0 then
        srv:close()
        return 0
    end

    local req = http.get("http://127.0.0.1:" .. tostring(port) .. "/ping", "HttpTest.on_client")
    for i = 1, 1500 do
        runtime.tick()
        if req.done then break end
        os.sleep(1)
    end
    if not req.done or req.err ~= nil then
        srv:close()
        return 0
    end
    if req.status ~= 200 then
        srv:close()
        return 0
    end
    if req.body ~= "hello:/ping" then
        srv:close()
        return 0
    end

    local post = http.post("http://127.0.0.1:" .. tostring(port) .. "/echo", "hi", "HttpTest.on_client")
    for i = 1, 1500 do
        runtime.tick()
        if post.done then break end
        os.sleep(1)
    end
    srv:close()
    if not post.done or post.err ~= nil then return 0 end
    if post.body ~= "echo:hi" then return 0 end
    return 1
end

function test_tls_echo()
    local cfg = {}
    cfg["ip"] = "127.0.0.1"
    cfg["port"] = 0
    cfg["tls"] = true
    cfg["cert"] = "./tls/cert.pem"
    cfg["key"] = "./tls/key.pem"
    local srv = http.server(cfg)
    srv:dispatch("HttpTest.on_server")
    local port = srv.port
    if port == nil or port == 0 then
        port = srv:get_port()
    end
    if port == nil or port == 0 then
        srv:close()
        return 0
    end

    local req = http.request({
        method = "GET",
        url = "https://127.0.0.1:" .. tostring(port) .. "/ping",
        tls_verify = false
    }, "HttpTest.on_client")
    for i = 1, 1500 do
        runtime.tick()
        if req.done then break end
        os.sleep(1)
    end
    srv:close()
    if not req.done or req.err ~= nil then return 0 end
    if req.status ~= 200 then return 0 end
    if req.body ~= "hello:/ping" then return 0 end
    return 1
end

function on_fail(req, err, resp)
    req.done = true
    req.err = err
end

function test_connect_fail()
    local cfg = {}
    cfg["method"] = "GET"
    cfg["url"] = "http://127.0.0.1:1/"
    cfg["timeout_ms"] = 500
    local req = http.request(cfg, "HttpTest.on_fail")
    for i = 1, 1500 do
        runtime.tick()
        if req.done then break end
        os.sleep(1)
    end
    if not req.done then return 0 end
    if type(req.err) ~= "string" or #req.err == 0 then return 0 end
    return 1
end
