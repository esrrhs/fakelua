function test_format_p_bad_tbl()
    -- string.format "%p" format requires a number argument; Table is invalid per standard Lua
    local t = {1, 2, 3}
    string.format("%p", t)
    return 0
end
