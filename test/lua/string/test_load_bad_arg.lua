function test_load_bad_arg()
    -- load arg #1 must be a string/number; Bool is invalid per standard Lua
    load(true)
    return 0
end
