package "JsonTest"

-- 测试 JSON 解析错误：尾随垃圾字符
function test_decode_trailing_garbage()
    local ok, err = pcall(function() json.decode('{"a":1}garbage') end)
    if ok then return 0 end
    if not string.find(err, "trailing garbage") then return 0 end
    return 1
end

-- 测试 JSON 解析错误：无效的转义字符
function test_decode_invalid_escape()
    local ok, err = pcall(function() json.decode('"\\q"') end)
    if ok then return 0 end
    if not string.find(err, "invalid escape") then return 0 end
    return 1
end

-- 测试 JSON 解析错误：无效的 null
function test_decode_invalid_null()
    local ok, err = pcall(function() json.decode('nulx') end)
    if ok then return 0 end
    if not string.find(err, "invalid null") then return 0 end
    return 1
end

-- 测试 JSON 解析错误：无效的 boolean (truex -> true + trailing garbage)
function test_decode_invalid_bool()
    local ok, err = pcall(function() json.decode('truex') end)
    if ok then return 0 end
    -- truex: parser reads "true" then finds trailing "x" -> "trailing garbage"
    if not string.find(err, "trailing garbage") then return 0 end
    return 1
end


-- 测试 JSON 解析错误：数字前导零
function test_decode_leading_zero()
    local ok, err = pcall(function() json.decode('01') end)
    if ok then return 0 end
    if not string.find(err, "invalid number") then return 0 end
    return 1
end

-- 测试 JSON 解析错误：小数点后无数字
function test_decode_dot_no_digit()
    local ok, err = pcall(function() json.decode('1.') end)
    if ok then return 0 end
    if not string.find(err, "invalid number") then return 0 end
    return 1
end

-- 测试 JSON 解析错误：e 后无数字
function test_decode_exp_no_digit()
    local ok, err = pcall(function() json.decode('1e+') end)
    if ok then return 0 end
    if not string.find(err, "invalid number") then return 0 end
    return 1
end

-- 测试 JSON 解析错误：空输入
function test_decode_empty_input()
    local ok, err = pcall(function() json.decode('') end)
    if ok then return 0 end
    return 1
end

-- 测试 JSON 解析错误：只有空白
function test_decode_whitespace_only()
    local ok, err = pcall(function() json.decode('   ') end)
    if ok then return 0 end
    return 1
end

-- 测试 JSON 解析错误：未终止的字符串
function test_decode_unterminated_string()
    local ok, err = pcall(function() json.decode('"hello') end)
    if ok then return 0 end
    if not string.find(err, "unterminated string") then return 0 end
    return 1
end

-- 测试 JSON 解析错误：未终止的数组
function test_decode_unterminated_array()
    local ok, err = pcall(function() json.decode('[1,2,3') end)
    if ok then return 0 end
    if not string.find(err, "unterminated array") then return 0 end
    return 1
end

-- 测试 JSON 解析错误：未终止的对象
function test_decode_unterminated_object()
    local ok, err = pcall(function() json.decode('{"a":1') end)
    if ok then return 0 end
    if not string.find(err, "unterminated object") then return 0 end
    return 1
end

-- 测试 JSON 解析错误：对象中非字符串键
function test_decode_non_string_key()
    local ok, err = pcall(function() json.decode('{123:"a"}') end)
    if ok then return 0 end
    if not string.find(err, "expected string key") then return 0 end
    return 1
end

-- 测试 JSON 解析错误：缺少冒号
function test_decode_missing_colon()
    local ok, err = pcall(function() json.decode('{"a" 1}') end)
    if ok then return 0 end
    return 1
end

-- 测试 JSON 解析错误：意外字符
function test_decode_unexpected_char()
    local ok, err = pcall(function() json.decode('@') end)
    if ok then return 0 end
    if not string.find(err, "unexpected character") then return 0 end
    return 1
end

-- 测试 JSON 字符串转义：反斜杠
function test_decode_escape_backslash()
    local v = json.decode('"a\\\\b"')
    if v ~= "a\\b" then return 0 end
    return 1
end
-- 测试 JSON 字符串转义：正斜杠
function test_decode_escape_slash()
    local v = json.decode('"a\\/b"')
    if v ~= "a/b" then return 0 end
    return 1
end

-- 测试 JSON 字符串转义：回车
function test_decode_escape_cr()
    local v = json.decode('"a\\rb"')
    if v ~= "a\rb" then return 0 end
    return 1
end

-- 测试 JSON 字符串转义：退格
function test_decode_escape_bs()
    local v = json.decode('"a\\bb"')
    if v ~= "a\bb" then return 0 end
    return 1
end

-- 测试 JSON 字符串转义：换页
function test_decode_escape_ff()
    local v = json.decode('"a\\fb"')
    if v ~= "a\fb" then return 0 end
    return 1
end

-- 测试 JSON 字符串转义：Unicode 基本多文种平面
function test_decode_escape_unicode()
    local v = json.decode('"\\u0041"')
    if v ~= "A" then return 0 end
    return 1
end

-- 测试 JSON 字符串转义：Unicode 中文
function test_decode_escape_unicode_chinese()
    local v = json.decode('"\\u4e2d\\u6587"')
    if v ~= "中文" then return 0 end
    return 1
end

-- 测试 JSON 字符串转义：Unicode 代理对（emoji）
function test_decode_escape_unicode_surrogate()
    local v = json.decode('"\\uD83D\\uDE00"')
    if v ~= "😀" then return 0 end
    return 1
end

-- 测试 JSON 字符串转义：无效的代理对
function test_decode_invalid_surrogate()
    local ok, err = pcall(function() json.decode('"\\uD800"') end)
    if ok then return 0 end
    if not string.find(err, "lone UTF-16 surrogate") then return 0 end
    return 1
end

-- 测试 JSON 字符串转义：无效的代理对低位
function test_decode_invalid_surrogate_low()
    local ok, err = pcall(function() json.decode('"\\uD800\\u0000"') end)
    if ok then return 0 end
    if not string.find(err, "invalid UTF-16 surrogate pair") then return 0 end
    return 1
end

-- 测试 JSON 字符串转义：单独的低代理
function test_decode_lone_low_surrogate()
    local ok, err = pcall(function() json.decode('"\\uDC00"') end)
    if ok then return 0 end
    if not string.find(err, "lone UTF-16 surrogate") then return 0 end
    return 1
end

-- 测试 JSON 编码：整数键（连续从1开始，应编码为数组）
function test_encode_int_key()
    local t = {[1] = "a", [2] = "b"}
    local s = json.encode(t)
    -- 连续整数键从1开始，应编码为JSON数组
    -- 验证是数组格式且包含 a 和 b
    if string.sub(s, 1, 1) ~= "[" then return 0 end
    if string.sub(s, -1, -1) ~= "]" then return 0 end
    if not string.find(s, "a") then return 0 end
    if not string.find(s, "b") then return 0 end
    return 1
end
-- 测试 JSON 编码：浮点数键
function test_encode_float_key()
    local t = {}
    t[1.5] = "x"
    local s = json.encode(t)
    -- 浮点数键应编码为对象
    if not string.find(s, "1.5") then return 0 end
    return 1
end

-- 测试 JSON 编码：布尔键
function test_encode_bool_key()
    local t = {}
    t[true] = "yes"
    t[false] = "no"
    local s = json.encode(t)
    if not string.find(s, "true") then return 0 end
    if not string.find(s, "false") then return 0 end
    return 1
end

-- 测试 JSON 编码：字符串中的特殊字符
function test_encode_special_chars()
    local s = json.encode('a"b\\c\nd\re\tf\bg')
    local decoded = json.decode(s)
    if decoded ~= 'a"b\\c\nd\re\tf\bg' then return 0 end
    return 1
end

-- 测试 JSON 编码：控制字符
function test_encode_control_chars()
    local s = json.encode(string.char(0x01, 0x1F))
    local decoded = json.decode(s)
    if decoded ~= string.char(0x01, 0x1F) then return 0 end
    return 1
end

-- 测试 JSON 编码：空对象
function test_encode_empty_object()
    local s = json.encode({})
    -- 空表应编码为空对象
    if s ~= "{}" then return 0 end
    return 1
end

-- 测试 JSON 编码：嵌套深度超限
function test_encode_nested_too_deep()
    local t = {}
    local cur = t
    for i = 1, 70 do
        cur[1] = {}
        cur = cur[1]
    end
    local ok, err = pcall(function() json.encode(t) end)
    if ok then return 0 end
    if not string.find(err, "nesting too deep") then return 0 end
    return 1
end

-- 测试 JSON 编码：不支持的类型（function）
function test_encode_unsupported_type()
    local f = function() end
    local ok, err = pcall(function() json.encode(f) end)
    if ok then return 0 end
    if not string.find(err, "unsupported type") then return 0 end
    return 1
end

-- 测试 JSON 编码：循环表
function test_encode_cyclic()
    local t = {}
    t.self = t
    local ok, err = pcall(function() json.encode(t) end)
    if ok then return 0 end
    if not string.find(err, "cyclic table") then return 0 end
    return 1
end

-- 测试 JSON 编码：非连续整数键（应编码为对象）
function test_encode_sparse_array()
    local t = {[1] = "a", [3] = "c"}
    local s = json.encode(t)
    -- 非连续整数键应编码为对象
    if not string.find(s, "1") then return 0 end
    if not string.find(s, "3") then return 0 end
    return 1
end

-- 测试 JSON 编码：大整数键（应编码为对象）
function test_encode_large_int_key()
    local t = {}
    t[9999999] = "big"
    local s = json.encode(t)
    if not string.find(s, "9999999") then return 0 end
    return 1
end