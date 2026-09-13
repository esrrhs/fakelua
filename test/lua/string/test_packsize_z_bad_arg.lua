function test_packsize_z_bad_arg()
    -- string.packsize "z" format requires a string argument; Bool is invalid per standard Lua
    string.packsize("z", true)
    return 0
end
