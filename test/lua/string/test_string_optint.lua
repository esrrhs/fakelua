-- luaL_optinteger / luaL_checkinteger：1.5 必须报错，不能截成 1。
-- math.mininteger 作为负下标不能把 len+pos+1 做成有符号溢出。
local function must_err(fn, needle)
    local ok, err = pcall(fn)
    if ok then return false end
    if needle and not string.find(tostring(err), needle, 1, true) then return false end
    return true
end

local function nvals(...)
    return select("#", ...)
end

function test_string_optint()
    if not must_err(function() string.find("hello", "l", 1.5) end, "integer") then return 1 end
    if not must_err(function() string.match("hello", "l", 1.5) end, "integer") then return 2 end
    if not must_err(function() string.gsub("aaa", "a", "b", 1.5) end, "integer") then return 3 end
    if not must_err(function() string.unpack("b", string.char(65), 1.5) end, "integer") then return 4 end

    -- 显式 nil 仍与省略等价
    local a = string.find("hello", "l", nil)
    if a ~= 3 then return 5 end
    local m = string.match("hello", "l+", nil)
    if m ~= "ll" then return 6 end
    local s, cnt = string.gsub("aaa", "a", "b", nil)
    if s ~= "bbb" or cnt ~= 3 then return 7 end
    local unpacked = string.unpack("b", string.char(65), nil)
    if unpacked ~= 65 then return 8 end

    -- mininteger 在起点之前：byte 返回 0 个值；sub/find 从 1 开始
    if nvals(string.byte("hi", math.mininteger)) ~= 0 then return 9 end
    if string.sub("hello", math.mininteger) ~= "hello" then return 10 end
    if string.find("hello", "h", math.mininteger) ~= 1 then return 11 end
    if string.match("hello", "h", math.mininteger) ~= "h" then return 12 end

    -- 格式串类型：缺参、nil、bool 都要报 string expected
    if not must_err(function() string.format() end, "string expected") then return 13 end
    if not must_err(function() string.format(nil) end, "string expected") then return 14 end
    if not must_err(function() string.format(true) end, "string expected") then return 15 end
    if string.format("%d", 7) ~= "7" then return 16 end

    return 5000
end
