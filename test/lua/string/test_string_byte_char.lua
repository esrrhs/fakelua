function test_string_byte_char()
    local b = string.byte("A")
    if b ~= 65 then return 0 end
    local s = string.char(65, 66, 67)
    if s ~= "ABC" then return 0 end
    return 500
end
