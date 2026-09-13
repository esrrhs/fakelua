function test_io_boundary_error()
    -- file:seek: 非法 whence 报错（标准 Lua 行为：invalid option）
    local f = io.tmpfile()
    f:seek("bad")
    f:close()
    return 5000
end
