function test_string_rep()
    local s = "abc"
    if string.rep(s, 3) ~= "abcabcabc" then return 0 end
    if string.rep(s, 3, "-") ~= "abc-abc-abc" then return 0 end
    return 300
end
