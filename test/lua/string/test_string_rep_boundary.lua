function test_string_rep_boundary()
    -- 1. count = 0
    if string.rep("abc", 0) ~= "" then return 0 end

    -- 2. count = 1
    if string.rep("abc", 1) ~= "abc" then return 0 end

    -- 3. 大 count
    local r = string.rep("a", 100)
    if #r ~= 100 then return 0 end

    -- 4. 含 sep 参数
    if string.rep("ab", 3, ",") ~= "ab,ab,ab" then return 0 end

    -- 5. sep 为空字符串
    if string.rep("ab", 3, "") ~= "ababab" then return 0 end

    -- 6. 空字符串重复
    if string.rep("", 10) ~= "" then return 0 end

    return 5000
end
