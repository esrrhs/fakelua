function test_utf8_codepoint()
    -- ASCII codepoint
    local a = utf8.codepoint("ABC", 1, 1)
    if a ~= 65 then return 0 end

    -- Multiple codepoints
    local b, c, d = utf8.codepoint("ABC", 1, 3)
    if b ~= 65 then return 0 end
    if c ~= 66 then return 0 end
    if d ~= 67 then return 0 end

    -- 2-byte char (é = U+00E9 = 233)
    local e = utf8.codepoint("é", 1, 1)
    if e ~= 233 then return 0 end

    -- 3-byte char (€ = U+20AC = 8364)
    local f = utf8.codepoint("€", 1, 1)
    if f ~= 8364 then return 0 end

    -- 4-byte char (𝄞 = U+1D11E = 119070)
    local g = utf8.codepoint("𝄞", 1, 1)
    if g ~= 119070 then return 0 end

    -- Default range (i defaults to 1, j defaults to i)
    local h = utf8.codepoint("X")
    if h ~= 88 then return 0 end

    return 6000
end
