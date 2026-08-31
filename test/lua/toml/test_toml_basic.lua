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

-- 测试解码 date 类型
function test_decode_date()
    local v = toml.decode("d = 1979-05-27")
    if type(v) ~= "table" then return 0 end
    -- date 被渲染为字符串
    if type(v.d) ~= "string" then return 0 end
    if v.d:find("1979") == nil then return 0 end
    return 1
end

-- 测试解码 time 类型
function test_decode_time()
    local v = toml.decode("t = 07:32:00")
    if type(v) ~= "table" then return 0 end
    if type(v.t) ~= "string" then return 0 end
    if v.t:find("32") == nil then return 0 end
    return 1
end

-- 测试解码 datetime 类型
function test_decode_datetime()
    local v = toml.decode("dt = 1979-05-27T07:32:00Z")
    if type(v) ~= "table" then return 0 end
    if type(v.dt) ~= "string" then return 0 end
    if v.dt:find("1979") == nil then return 0 end
    return 1
end

-- 测试 encode 嵌套表
function test_encode_nested_table()
    local orig = {db = {server = "192.168.1.1", ports = {8001, 8002}, conn_max = 5000}}
    local s = toml.encode(orig)
    if type(s) ~= "string" then return 0 end
    if s:find("192.168.1.1") == nil then return 0 end
    if s:find("8001") == nil then return 0 end
    return 1
end

-- 测试 encode 布尔值和浮点数
function test_encode_bool_float()
    local orig = {flag = false, value = 2.718}
    local s = toml.encode(orig)
    if type(s) ~= "string" then return 0 end
    if s:find("false") == nil then return 0 end
    if s:find("2.718") == nil then return 0 end
    return 1
end

-- 测试 encode 数组（含子表）
function test_encode_array_with_tables()
    local orig = {products = {{name = "A"}, {name = "B"}}}
    local s = toml.encode(orig)
    if type(s) ~= "string" then return 0 end
    if s:find("name") == nil then return 0 end
    if s:find("A") == nil then return 0 end
    return 1
end

-- 测试 encode 顶层标量
function test_encode_top_scalar()
    local s = toml.encode(42)
    if type(s) ~= "string" then return 0 end
    if s:find("42") == nil then return 0 end
    return 1
end

-- 测试 encode 空表
function test_encode_empty_table()
    local s = toml.encode({})
    if type(s) ~= "string" then return 0 end
    return 1
end

-- 测试 encode 循环表（应抛异常）
function test_encode_cyclic()
    local t = {}
    t.self = t
    local ok, err = pcall(function() toml.encode(t) end)
    if ok then return 0 end
    return 1
end
