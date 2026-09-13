function test_dofile_bad_arg()
    -- dofile arg #1 must be a string; Bool is invalid per standard Lua
    dofile(true)
    return 0
end
