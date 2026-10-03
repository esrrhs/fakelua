function test_gsub_bad_func_return_bool()
    -- Lua 5.4 语义：替换函数返回 nil 或 false 时保留原匹配（不报错、不删除）。
    local r1, c1 = string.gsub("hello", "l", function()
        return false
    end)
    if r1 ~= "hello" or c1 ~= 2 then return 0 end

    local r2, c2 = string.gsub("abc", "b", function()
        return nil
    end)
    if r2 ~= "abc" or c2 ~= 1 then return 0 end

    -- 返回 true（非 nil/false 的布尔）仍然是非法替换值
    local ok = pcall(function()
        string.gsub("a", "a", function()
            return true
        end)
    end)
    if ok then return 0 end

    return 1
end
