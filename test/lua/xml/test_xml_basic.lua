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

-- 测试 encode 带多种属性
function test_encode_multi_attrs()
    local s = xml.encode({person = {["_attr_name"] = "Bob", ["_attr_age"] = "25", ["_attr_city"] = "NYC"}})
    if type(s) ~= "string" then return 0 end
    if s:find("Bob", 1, true) == nil then return 0 end
    if s:find("25", 1, true) == nil then return 0 end
    if s:find("NYC", 1, true) == nil then return 0 end
    return 1
end

-- 测试 encode 带 _text 节点
function test_encode_text_node()
    local s = xml.encode({msg = {["_text"] = "hello world"}})
    if type(s) ~= "string" then return 0 end
    if s:find("hello world", 1, true) == nil then return 0 end
    return 1
end

-- 测试 encode 数值类型（int/float/bool）
function test_encode_scalar_types()
    local s = xml.encode({root = {intval = 42, floatval = 3.14, boolval = true}})
    if type(s) ~= "string" then return 0 end
    if s:find("42", 1, true) == nil then return 0 end
    if s:find("3.14", 1, true) == nil then return 0 end
    if s:find("true", 1, true) == nil then return 0 end
    return 1
end

-- 测试 encode 数组（连续整数键）
function test_encode_array()
    local s = xml.encode({list = {{["_text"] = "a"}, {["_text"] = "b"}, {["_text"] = "c"}}})
    if type(s) ~= "string" then return 0 end
    -- 数组应该生成多个 item 元素，且包含 a/b/c 文本
    if s:find("item", 1, true) == nil then return 0 end
    if s:find("a", 1, true) == nil then return 0 end
    if s:find("b", 1, true) == nil then return 0 end
    if s:find("c", 1, true) == nil then return 0 end
    return 1
end

-- 测试 encode 循环表（应抛异常）
function test_encode_cyclic()
    local t = {}
    t.self = t  -- 循环引用
    local ok, err = pcall(function() xml.encode(t) end)
    if ok then return 0 end
    return 1
end

-- 测试 encode 嵌套表
function test_encode_nested()
    local s = xml.encode({root = {child = {grandchild = {["_text"] = "deep"}}}})
    if type(s) ~= "string" then return 0 end
    if s:find("deep", 1, true) == nil then return 0 end
    return 1
end

-- 测试 encode 顶层标量（非表）
function test_encode_top_scalar()
    local s = xml.encode(42)
    if type(s) ~= "string" then return 0 end
    if s:find("42", 1, true) == nil then return 0 end
    return 1
end

-- 测试 encode 空表
function test_encode_empty_table()
    local s = xml.encode({})
    if type(s) ~= "string" then return 0 end
    return 1
end

-- 回归测试：非顶层 array 必须挂在其所属元素下，而不是父节点的兄弟位置
function test_encode_nested_array_parent()
    local s = xml.encode({root = {list = {{a = "x"}, {a = "y"}}}})
    if type(s) ~= "string" then return 0 end
    -- 期望 <list><item>...</item><item>...</item></list>，而不是 <list/> 后接兄弟 <item>
    -- 简单校验 </list> 出现在最后一个 </item> 之后（说明 item 在 list 内）
    local last_item = 0
    local pos = 1
    while true do
        local a, b = s:find("</item>", pos, true)
        if not a then break end
        last_item = a
        pos = b + 1
    end
    local list_close = s:find("</list>", 1, true)
    if last_item == 0 or list_close == nil then return 0 end
    if list_close < last_item then return 0 end
    -- 且不应出现 <list /> 或 <list/> 自闭合
    if s:find("<list ?/>", 1, false) ~= nil then return 0 end
    return 1
end
