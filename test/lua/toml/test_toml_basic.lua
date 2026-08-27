package "TomlTest"

function test_decode_int()
    local v = toml.decode("age = 30")
    if type(v) ~= "table" then return 0 end
    if v.age ~= 30 then return 0 end
    return 1
end

function test_decode_float()
    local v = toml.decode("pi = 3.14")
    if type(v) ~= "table" then return 0 end
    if v.pi < 3.13 or v.pi > 3.15 then return 0 end
    return 1
end

function test_decode_bool()
    local v = toml.decode("flag = true")
    if type(v) ~= "table" then return 0 end
    if v.flag ~= true then return 0 end
    return 1
end

function test_decode_string()
    local v = toml.decode('name = "Alice"')
    if type(v) ~= "table" then return 0 end
    if v.name ~= "Alice" then return 0 end
    return 1
end

function test_decode_array()
    local v = toml.decode("nums = [1, 2, 3]")
    if type(v) ~= "table" then return 0 end
    if type(v.nums) ~= "table" then return 0 end
    if v.nums[1] ~= 1 or v.nums[2] ~= 2 or v.nums[3] ~= 3 then return 0 end
    return 1
end

function test_decode_table()
    local v = toml.decode("[db]\nhost = \"localhost\"\nport = 3306\n")
    if type(v) ~= "table" then return 0 end
    if type(v.db) ~= "table" then return 0 end
    if v.db.host ~= "localhost" then return 0 end
    if v.db.port ~= 3306 then return 0 end
    return 1
end

function test_decode_error()
    local ok, msg = pcall(function() toml.decode("key = ") end)
    if ok then return 0 end  -- incomplete value should error
    return 1
end

function test_encode_basic()
    local s = toml.encode({title = "test", count = 5})
    if type(s) ~= "string" then return 0 end
    if s:find("title", 1, true) == nil then return 0 end
    if s:find("count", 1, true) == nil then return 0 end
    return 1
end

function test_roundtrip()
    local orig = {name = "Alice", age = 30, tags = {"a", "b"}}
    local s = toml.encode(orig)
    local t = toml.decode(s)
    if t.name ~= "Alice" then return 0 end
    if t.age ~= 30 then return 0 end
    if type(t.tags) ~= "table" then return 0 end
    if t.tags[1] ~= "a" or t.tags[2] ~= "b" then return 0 end
    return 1
end
