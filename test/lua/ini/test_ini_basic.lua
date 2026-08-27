package "IniTest"

function test_decode_basic()
    local t = ini.decode("[section1]\nkey1 = value1\nkey2 = 42\n")
    if type(t) ~= "table" then return 0 end
    if type(t.section1) ~= "table" then return 0 end
    if t.section1.key1 ~= "value1" then return 0 end
    if t.section1.key2 ~= 42 then return 0 end
    return 1
end

function test_decode_multiple_sections()
    local t = ini.decode("[a]\nx = 1\n[b]\ny = 2\n")
    if type(t) ~= "table" then return 0 end
    if t.a.x ~= 1 then return 0 end
    if t.b.y ~= 2 then return 0 end
    return 1
end

function test_decode_types()
    local t = ini.decode("[s]\nbool1 = true\nbool2 = false\nnum = 3.14\nstr = hello\n")
    if type(t) ~= "table" then return 0 end
    if t.s.bool1 ~= true then return 0 end
    if t.s.bool2 ~= false then return 0 end
    if t.s.num < 3.13 or t.s.num > 3.15 then return 0 end
    if t.s.str ~= "hello" then return 0 end
    return 1
end

function test_decode_empty()
    local t = ini.decode("")
    if type(t) ~= "table" then return 0 end
    return 1
end

function test_encode_basic()
    local t = {section1 = {key1 = "value1", key2 = 42}}
    local s = ini.encode(t)
    if type(s) ~= "string" then return 0 end
    -- plain text search (4th arg true disables pattern matching)
    if s:find("[section1]", 1, true) == nil then return 0 end
    if s:find("key1 = value1", 1, true) == nil then return 0 end
    if s:find("key2 = 42", 1, true) == nil then return 0 end
    return 1
end

function test_roundtrip()
    local orig = "[db]\nhost = 127.0.0.1\nport = 3306\n"
    local t = ini.decode(orig)
    if type(t) ~= "table" then return 0 end
    if t.db.host ~= "127.0.0.1" then return 0 end
    if t.db.port ~= 3306 then return 0 end
    local enc = ini.encode(t)
    if type(enc) ~= "string" then return 0 end
    -- re-decode the encoded output and verify values survive
    local t2 = ini.decode(enc)
    if t2.db.host ~= "127.0.0.1" then return 0 end
    if t2.db.port ~= 3306 then return 0 end
    return 1
end
