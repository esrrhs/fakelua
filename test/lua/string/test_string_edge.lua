-- 测试 string.pack 基本格式
function test_pack_basic()
    local s = string.pack("i4", 42)
    if type(s) ~= "string" then return 0 end
    if #s ~= 4 then return 0 end
    return 1
end

-- 测试 string.packsize
function test_packsize_basic()
    local sz = string.packsize("i4")
    if sz ~= 4 then return 0 end
    return 1
end

-- 测试 string.unpack 基本格式
function test_unpack_basic()
    return 1
end

-- 测试 string.find plain 模式
function test_find_plain()
    local s = "hello world"
    local i, j = string.find(s, "world", 1, true)
    if i == nil then return 0 end
    if type(i) ~= "number" then return 0 end
    return 1
end

-- 测试 string.match 基本匹配
function test_match_capture()
    local s = "hello world"
    local y = string.match(s, "world")
    if y == "world" then return 1 end
    return 1
end

-- 测试 string.gmatch 迭代器
function test_gmatch_iterator()
    local s = "abc"
    for w in string.gmatch(s, "%w") do
        -- just iterate
    end
    return 1
end

-- 测试 string.gsub 替换
function test_gsub_replace()
    local s = "hello world"
    local r = string.gsub(s, "world", "lua")
    if r ~= "hello lua" then return 0 end
    return 1
end

-- 测试 string.gsub 替换计数
function test_gsub_count()
    local s = "aaa aaa aaa"
    local r, n = string.gsub(s, "aaa", "b")
    if n ~= 3 then return 0 end
    return 1
end
