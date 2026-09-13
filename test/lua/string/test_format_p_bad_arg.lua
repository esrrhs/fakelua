function test_format_p_bad_arg()
    -- string.format "%p" format requires a number argument; Bool is invalid per standard Lua
    string.format("%p", true)
    return 0
end
