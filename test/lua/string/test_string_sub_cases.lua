package "StringSubCases"

-- 测试 string.sub 负数索引
function test_sub_negative()
    local s = "Hello World"
    local sub = string.sub(s, -5, -1)
    if sub ~= "World" then return 0 end
    return 1
end

-- 测试 string.sub 部分范围
function test_sub_partial()
    local s = "Hello World"
    local sub = string.sub(s, 1, 5)
    if sub ~= "Hello" then return 0 end
    return 1
end

-- 测试 string.sub 超出范围
function test_sub_out_of_range()
    local s = "Hi"
    local sub = string.sub(s, 1, 100)
    if sub ~= "Hi" then return 0 end
    return 1
end
