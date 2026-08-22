package "JsonTest"

function test_decode_null()
    local v = json.decode("null")
    if v ~= nil then return 0 end
    return 1
end

function test_decode_bool()
    local t = json.decode("true")
    local f = json.decode("false")
    if t ~= true or f ~= false then return 0 end
    return 1
end

function test_decode_int()
    local v = json.decode("42")
    if v ~= 42 then return 0 end
    return 1
end

function test_decode_negative()
    local v = json.decode("-100")
    if v ~= -100 then return 0 end
    return 1
end

function test_decode_float()
    local v = json.decode("3.14")
    if v < 3.13 or v > 3.15 then return 0 end
    return 1
end

function test_decode_string()
    local v = json.decode('"hello"')
    if v ~= "hello" then return 0 end
    return 1
end

function test_decode_string_escape()
    local v = json.decode('"a\\nb\\tc"')
    if v ~= "a\nb\tc" then return 0 end
    return 1
end

function test_decode_array()
    local v = json.decode("[1, 2, 3]")
    if type(v) ~= "table" then return 0 end
    if v[1] ~= 1 or v[2] ~= 2 or v[3] ~= 3 then return 0 end
    return 1
end

function test_decode_object()
    local v = json.decode('{"a": 1, "b": "hello"}')
    if type(v) ~= "table" then return 0 end
    if v.a ~= 1 or v.b ~= "hello" then return 0 end
    return 1
end

function test_decode_nested()
    local v = json.decode('{"a": [1, 2, {"b": true}]}')
    if type(v) ~= "table" then return 0 end
    if type(v.a) ~= "table" then return 0 end
    if v.a[1] ~= 1 or v.a[2] ~= 2 then return 0 end
    if type(v.a[3]) ~= "table" then return 0 end
    if v.a[3].b ~= true then return 0 end
    return 1
end

function test_encode_null()
    local s = json.encode(nil)
    if s ~= "null" then return 0 end
    return 1
end

function test_encode_bool()
    local t = json.encode(true)
    local f = json.encode(false)
    if t ~= "true" or f ~= "false" then return 0 end
    return 1
end

function test_encode_int()
    local s = json.encode(42)
    if s ~= "42" then return 0 end
    return 1
end

function test_encode_float()
    local s = json.encode(3.14)
    -- 允许不同的浮点表示，只要数值正确即可
    local v = json.decode(s)
    if v < 3.13 or v > 3.15 then return 0 end
    return 1
end

function test_encode_string()
    local s = json.encode("hello")
    if s ~= '"hello"' then return 0 end
    return 1
end

function test_encode_array()
    local s = json.encode({1, 2, 3})
    if s ~= "[1,2,3]" then return 0 end
    return 1
end

function test_encode_object()
    local s = json.encode({a=1, b="hello"})
    -- 对象键顺序不固定，检查两种可能
    if s ~= '{"a":1,"b":"hello"}' and s ~= '{"b":"hello","a":1}' then return 0 end
    return 1
end

function test_roundtrip()
    local orig = {name="明朝", provinces=3, data={1, 2, 3}, active=true}
    local encoded = json.encode(orig)
    local decoded = json.decode(encoded)
    if decoded.name ~= "明朝" then return 0 end
    if decoded.provinces ~= 3 then return 0 end
    if type(decoded.data) ~= "table" then return 0 end
    if decoded.data[1] ~= 1 or decoded.data[2] ~= 2 or decoded.data[3] ~= 3 then return 0 end
    if decoded.active ~= true then return 0 end
    return 1
end
