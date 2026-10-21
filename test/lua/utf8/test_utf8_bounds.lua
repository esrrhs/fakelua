-- utf8 与 Lua 5.4 对齐：
--   codepoint 空串 / 越界 / 非法字节是错误，不是 nil
--   len 的 i/j 越界是错误；非法字节仍返回 nil + 位置
--   i/j/n 的 1.5 不能截成整数
local function must_err(fn, needle)
    local ok, err = pcall(fn)
    if ok then return false end
    if needle and not string.find(tostring(err), needle, 1, true) then return false end
    return true
end

local function nvals(...)
    return select("#", ...)
end

function test_utf8_bounds()
    if not must_err(function() utf8.codepoint("") end, "out of bounds") then return 1 end
    if not must_err(function() utf8.codepoint("ab", 3) end, "out of bounds") then return 2 end
    if not must_err(function() utf8.codepoint("ab", 1, 3) end, "out of bounds") then return 3 end
    if not must_err(function() utf8.codepoint("\255") end, "invalid UTF-8") then return 4 end
    -- i > j 且 j 仍在串内：返回 0 个值，不报错
    if nvals(utf8.codepoint("ab", 2, 1)) ~= 0 then return 5 end
    if utf8.codepoint("A") ~= 65 then return 6 end

    if not must_err(function() utf8.len("", 2) end, "out of bounds") then return 7 end
    if not must_err(function() utf8.len("a", 3) end, "out of bounds") then return 8 end
    if not must_err(function() utf8.len("a", 1, 2) end, "out of bounds") then return 9 end
    if utf8.len("") ~= 0 then return 10 end
    local n, pos = utf8.len("\255")
    if n ~= nil or pos ~= 1 then return 11 end

    if not must_err(function() utf8.codepoint("ab", 1.5) end, "integer") then return 12 end
    if not must_err(function() utf8.len("ab", 1.5) end, "integer") then return 13 end
    if not must_err(function() utf8.offset("ab", 1.5) end, "integer") then return 14 end
    if not must_err(function() utf8.offset("ab", 1, 1.5) end, "integer") then return 15 end

    -- mininteger 不能溢出；落在起点之前就是越界
    if not must_err(function() utf8.codepoint("ab", math.mininteger) end, "out of bounds") then return 16 end
    if not must_err(function() utf8.offset("ab", 1, math.mininteger) end, "out of range") then return 17 end

    return 5000
end
