function test_url_contracts()
    local formatted = url.format({scheme = "https", user = "alice", password = "secret", host = "example.invalid", port = "8443", path = "/a", query = "x=1", fragment = "tail"})
    assert(formatted == "https://alice:secret@example.invalid:8443/a?x=1#tail")
    local query = url.decode_query("flag&x=1")
    assert(query.flag == "" and query.x == "1")
    assert(not pcall(function() url.decode("%GG") end))
    assert(not pcall(function() url.decode_query("x=%GG") end))
    assert(not pcall(function() url.parse("http://[") end))
    assert(not pcall(function() url.format(false) end))
    assert(not pcall(function() url.encode_query(false) end))
    return 1
end
