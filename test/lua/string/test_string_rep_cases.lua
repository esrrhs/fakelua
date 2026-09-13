package "StringRepCases"

-- 测试 string.rep 带分隔符
function test_rep_with_sep()
    local s = string.rep("ab", 3, ",")
    if s ~= "ab,ab,ab" then return 0 end
    return 1
end

-- 测试 string.rep 零次
function test_rep_zero()
    local s = string.rep("abc", 0)
    if s ~= "" then return 0 end
    return 1
end
