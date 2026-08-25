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
    if string.rep("", math.maxinteger) ~= "" then return 0 end
    if string.rep("", math.maxinteger, "") ~= "" then return 0 end

    -- 7. 恰好是整数的 float 仍合法
    if string.rep("ab", 3.0) ~= "ababab" then return 0 end

    return 5000
end

function test_string_rep_2pow63()
    string.rep("x", 2^63)
end

function test_string_rep_nan()
    string.rep("x", 0/0)
end

function test_string_rep_frac()
    string.rep("x", 1.5)
end
