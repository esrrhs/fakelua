function test_utf8_invalid_sequences()
    local invalid = {string.char(0xc2), string.char(0xe2, 0x28, 0xa1),
        string.char(0xe0, 0x80, 0x80), string.char(0xed, 0xa0, 0x80),
        string.char(0xf0, 0x80, 0x80, 0x80), string.char(0xf4, 0x90, 0x80, 0x80),
        string.char(0xf0, 0x90, 0x80)}
    for _, text in ipairs(invalid) do
        local length, position = utf8.len(text)
        assert(length == nil and position == 1)
        assert(not pcall(function() utf8.codepoint(text) end))
        local iterator, state, control = utf8.codes(text)
        assert(not pcall(function() iterator(state, control) end))
    end
    local text = utf8.char(0x20ac)
    assert(not pcall(function() utf8.offset(text, 1, 2) end))
    return 1
end
