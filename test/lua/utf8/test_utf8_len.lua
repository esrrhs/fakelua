function test_utf8_len()
    -- ASCII string
    local l1 = utf8.len("hello")
    if l1 ~= 5 then return 0 end

    -- Mixed: "aé€" = 1 + 1 + 1 = 3 UTF-8 chars (but different byte lengths)
    local l2 = utf8.len("aé€")
    if l2 ~= 3 then return 0 end

    -- Empty string
    local l3 = utf8.len("")
    if l3 ~= 0 then return 0 end

    -- With range
    local l4 = utf8.len("hello", 2, 4)
    if l4 ~= 3 then return 0 end

    -- 4-byte char
    local l5 = utf8.len("𝄞")
    if l5 ~= 1 then return 0 end

    return 6000
end
