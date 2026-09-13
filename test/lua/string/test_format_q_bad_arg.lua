function test_format_q_bad_arg()
    -- string.format "%q" format requires a string argument; Table is invalid per standard Lua
    string.format("%q", {})
    return 0
end
