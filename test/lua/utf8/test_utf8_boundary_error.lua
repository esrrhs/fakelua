-- Test utf8 error paths: wrong argument types must throw
-- Covers: utf8.char / utf8.codepoint / utf8.len / utf8.offset type validation

function test_utf8_char_bad_arg()
    utf8.char("bad")
end

function test_utf8_codepoint_bad_arg()
    utf8.codepoint(123)
end

function test_utf8_len_bad_arg()
    utf8.len(123)
end

function test_utf8_offset_bad_arg()
    utf8.offset(123, 1)
end
