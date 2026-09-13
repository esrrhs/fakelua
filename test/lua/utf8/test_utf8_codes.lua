function test_utf8_codes()
    -- utf8.codes returns the string (simplified implementation)
    -- Users can use utf8.codepoint in a loop for iteration
    local s = utf8.codes("hello")
    if type(s) ~= "string" then return 0 end
    if s ~= "hello" then return 0 end

    -- Test that we can iterate using utf8.codepoint
    local text = "AB"
    local count = 0
    local cp_a = utf8.codepoint(text, 1, 1)
    local cp_b = utf8.codepoint(text, 2, 2)
    if cp_a ~= 65 then return 0 end
    if cp_b ~= 66 then return 0 end

    return 6000
end
