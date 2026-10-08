-- Lua 5.4: string.format("%s", x) 等价于对 x 调用 tostring（luaL_tolstring）：
-- string/number/bool/nil 均合法，只有 table（无 __tostring 元方法）报错。
function test_format_s_types()
    if string.format("%s", true) ~= "true" then return 1 end
    if string.format("%s", false) ~= "false" then return 2 end
    if string.format("%s", nil) ~= "nil" then return 3 end
    if string.format("%s", 42) ~= "42" then return 4 end
    -- float 走 Lua 数字格式，而不是 C 的 6 位小数
    if string.format("%s", 2.5) ~= "2.5" then return 5 end
    if string.format("%s", 1.0) ~= "1.0" then return 6 end
    if string.format("%s", 1e20) ~= "1e+20" then return 7 end
    if string.format("[%s]", 2.5) ~= "[2.5]" then return 8 end
    -- 宽度/对齐修饰仍作用于 tostring 的结果
    if string.format("%-6s|", "ab") ~= "ab    |" then return 9 end
    -- string 原样返回
    if string.format("%s", "hi") ~= "hi" then return 10 end

    -- table 没有 __tostring，仍然报错
    local ok = pcall(function() return string.format("%s", { 1, 2 }) end)
    if ok then return 11 end

    return 5000
end
