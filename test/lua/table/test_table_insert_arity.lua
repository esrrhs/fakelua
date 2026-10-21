-- table.insert 的参数个数与 Lua 5.4 一致：恰好 2 或 3 个。
local function must_err(fn, needle)
    local ok, err = pcall(fn)
    if ok then return false end
    if needle and not string.find(tostring(err), needle, 1, true) then return false end
    return true
end

function test_table_insert_arity()
    local t = {}
    if not must_err(function() table.insert(t) end, "wrong number of arguments") then return 1 end
    if not must_err(function() table.insert(t, 1, 2, 3) end, "wrong number of arguments") then return 2 end
    -- 类型错误仍优先于个数：非表不会被当成 wrong number
    if not must_err(function() table.insert(true) end, "table expected") then return 3 end

    table.insert(t, 10)
    table.insert(t, 1, 5)
    if t[1] ~= 5 or t[2] ~= 10 then return 4 end
    return 5000
end
