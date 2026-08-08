function test_format_s_bad_arg()
    -- string.format "%s" format requires a string/number argument; Bool is invalid per standard Lua
    string.format("%s", true)
    return 0
end
