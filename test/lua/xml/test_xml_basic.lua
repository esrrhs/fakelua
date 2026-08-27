package "XmlTest"

function test_decode_element()
    local t = xml.decode("<root>hello</root>")
    if type(t) ~= "table" then return 0 end
    if t["_text"] ~= "hello" then return 0 end
    return 1
end

function test_decode_attribute()
    local t = xml.decode('<person name="Alice" age="30"/>')
    if type(t) ~= "table" then return 0 end
    if t["_attr_name"] ~= "Alice" then return 0 end
    if t["_attr_age"] ~= "30" then return 0 end
    return 1
end

function test_decode_nested()
    local t = xml.decode("<root><child>value</child></root>")
    if type(t) ~= "table" then return 0 end
    if type(t.child) ~= "table" then return 0 end
    if t.child["_text"] ~= "value" then return 0 end
    return 1
end

function test_decode_error()
    local ok, msg = pcall(function() xml.decode("<unclosed") end)
    if ok then return 0 end
    return 1
end

function test_encode_basic()
    local s = xml.encode({root = {name = "test"}})
    if type(s) ~= "string" then return 0 end
    if s:find("root", 1, true) == nil then return 0 end
    if s:find("test", 1, true) == nil then return 0 end
    return 1
end

function test_roundtrip()
    -- xml.decode returns the root element directly as a table (not wrapped)
    local orig = {person = {["_attr_name"] = "Alice", age = "30"}}
    local s = xml.encode(orig)
    local t = xml.decode(s)
    if type(t) ~= "table" then return 0 end
    -- t IS the person element: has _attr_name and child age
    if t["_attr_name"] ~= "Alice" then return 0 end
    if type(t.age) ~= "table" then return 0 end
    if t.age["_text"] ~= "30" then return 0 end
    return 1
end
