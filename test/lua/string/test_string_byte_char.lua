function test_string_byte_char()
    local b = string.byte("A")
    if b ~= 65 then return 0 end
    local s = string.char(65, 66, 67)
    if s ~= "ABC" then return 0 end

    -- 验证 i > j 或越界时返回 0 个返回值 (select("#", ...) == 0)
    local empty_cnt = select("#", string.byte("abc", 5, 4))
    if empty_cnt ~= 0 then return 0 end

    return 500
end
