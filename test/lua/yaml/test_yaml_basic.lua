package "YamlTest"

function test_decode_scalar_int()
    local v = yaml.decode("42")
    if v ~= 42 then return 0 end
    return 1
end

function test_decode_scalar_float()
    local v = yaml.decode("3.14")
    if type(v) ~= "number" then return 0 end
    if v < 3.13 or v > 3.15 then return 0 end
    return 1
end

function test_decode_scalar_bool()
    if yaml.decode("true") ~= true then return 0 end
    if yaml.decode("false") ~= false then return 0 end
    return 1
end

function test_decode_scalar_null()
    local v = yaml.decode("null")
    if v ~= nil then return 0 end
    return 1
end

function test_decode_scalar_string()
    local v = yaml.decode("hello")
    if v ~= "hello" then return 0 end
    return 1
end

function test_decode_map()
    local t = yaml.decode("name: Alice\nage: 30\n")
    if type(t) ~= "table" then return 0 end
    if t.name ~= "Alice" then return 0 end
    if t.age ~= 30 then return 0 end
    return 1
end

function test_decode_array()
    local t = yaml.decode("- 1\n- 2\n- 3\n")
    if type(t) ~= "table" then return 0 end
    if t[1] ~= 1 or t[2] ~= 2 or t[3] ~= 3 then return 0 end
    return 1
end

function test_decode_nested()
    local t = yaml.decode("db:\n  host: localhost\n  port: 3306\n")
    if type(t) ~= "table" then return 0 end
    if type(t.db) ~= "table" then return 0 end
    if t.db.host ~= "localhost" then return 0 end
    if t.db.port ~= 3306 then return 0 end
    return 1
end

function test_decode_error()
    local ok, msg = pcall(function() yaml.decode("{{{invalid") end)
    if ok then
        return 1
    end
    return 1
end

function test_encode_scalar()
    local s = yaml.encode(42)
    if type(s) ~= "string" then return 0 end
    if s:find("42", 1, true) == nil then return 0 end
    return 1
end

function test_encode_map()
    local s = yaml.encode({name = "Alice", age = 30})
    if type(s) ~= "string" then return 0 end
    if s:find("Alice", 1, true) == nil then return 0 end
    if s:find("30", 1, true) == nil then return 0 end
    return 1
end

function test_roundtrip()
    local orig = {host = "localhost", port = 3306, tags = {1, 2, 3}}
    local s = yaml.encode(orig)
    local t = yaml.decode(s)
    if t.host ~= "localhost" then return 0 end
    if t.port ~= 3306 then return 0 end
    if type(t.tags) ~= "table" then return 0 end
    if t.tags[1] ~= 1 or t.tags[2] ~= 2 or t.tags[3] ~= 3 then return 0 end
    return 1
end
