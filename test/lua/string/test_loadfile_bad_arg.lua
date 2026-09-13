function test_loadfile_bad_arg()
    -- loadfile arg #1 must be a string; Bool is invalid per standard Lua
    loadfile(true)
    return 0
end
