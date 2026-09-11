package "UrlTest"

function test_parse()
    local u = url.parse("https://user:pass@example.com:8080/a/b?x=1&y=2#frag")
    if u.scheme ~= "https" then return 0 end
    if u.user ~= "user" then return 0 end
    if u.password ~= "pass" then return 0 end
    if u.host ~= "example.com" then return 0 end
    if u.port ~= 8080 then return 0 end
    if u.path ~= "/a/b" then return 0 end
    if u.query ~= "x=1&y=2" then return 0 end
    if u.fragment ~= "frag" then return 0 end
    if u.params.x ~= "1" or u.params.y ~= "2" then return 0 end
    return 1
end

function test_format()
    local t = {}
    t["scheme"] = "http"
    t["host"] = "127.0.0.1"
    t["port"] = 80
    t["path"] = "/ping"
    local params = {}
    params["q"] = "ok"
    t["params"] = params
    local s = url.format(t)
    if not string.find(s, "http", 1, true) then return 0 end
    if not string.find(s, "127.0.0.1", 1, true) then return 0 end
    if not string.find(s, "/ping", 1, true) then return 0 end
    return 1
end

function test_encode_decode()
    local e = url.encode("a b/c")
    if e ~= "a%20b%2Fc" then return 0 end
    local d = url.decode("a%20b")
    if d ~= "a b" then return 0 end
    local qtbl = {}
    qtbl["a"] = "1"
    qtbl["b"] = "x y"
    local q = url.encode_query(qtbl)
    local t = url.decode_query(q)
    if t.a ~= "1" then return 0 end
    if t.b ~= "x y" then return 0 end
    return 1
end
