function test_format_s_bad_arg_table()
    -- string.format "%s" format requires a string/number argument; Table is invalid per standard Lua
    local t = {1, 2, 3}
    string.format("%s", t)
    return 0
end
