function test_utf8_char()
    -- ASCII
    local s1 = utf8.char(65)
    if s1 ~= "A" then return 0 end

    -- Multiple ASCII chars
    local s2 = utf8.char(72, 101, 108, 108, 111)
    if s2 ~= "Hello" then return 0 end

    -- 2-byte UTF-8 (é = U+00E9)
    local s3 = utf8.char(233)
    if #s3 ~= 2 then return 0 end

    -- 3-byte UTF-8 (€ = U+20AC)
    local s4 = utf8.char(8364)
    if #s4 ~= 3 then return 0 end

    -- 4-byte UTF-8 (𝄞 = U+1D11E)
    local s5 = utf8.char(0x1D11E)
    if #s5 ~= 4 then return 0 end

    -- Empty string (no args)
    local s6 = utf8.char()
    if s6 ~= "" then return 0 end

    return 6000
end
