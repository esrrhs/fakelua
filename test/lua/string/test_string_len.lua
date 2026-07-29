function test_string_len()
    local s1 = "hello"
    local s2 = "fakelua world"
    if string.len(s1) ~= 5 then return 0 end
    if string.len(s2) ~= 13 then return 0 end
    return 100
end
