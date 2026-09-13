function test_string_len_bad_arg()
    -- string.len arg #1 must be a string; Bool is invalid per standard Lua
    string.len(true)
    return 0
end
