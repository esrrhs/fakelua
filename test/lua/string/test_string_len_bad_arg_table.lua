function test_string_len_bad_tbl()
    -- string.len arg #1 must be a string; Table is invalid per standard Lua
    local t = {1, 2, 3}
    string.len(t)
    return 0
end
