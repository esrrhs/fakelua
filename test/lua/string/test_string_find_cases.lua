package "StringFindCases"

-- 测试 string.find 正则捕获（fakelua 使用 ECMAScript 正则语法）
function test_find_capture()
    local s = "hello world 123"
    local start, finish, cap = string.find(s, "(\\d+)")
    if start == nil then return 0 end
    if cap ~= "123" then return 0 end
    return 1
end

-- 测试 string.find plain 模式
function test_find_plain_mode()
    local s = "hello world"
    local i = string.find(s, "world", 1, true)
    if i == nil then return 0 end
    return 1
end

-- 测试 string.find 找不到
function test_find_not_found()
    local s = "hello"
    local i = string.find(s, "xyz")
    if i ~= nil then return 0 end
    return 1
end
