function test_io_tmpfile()
    -- 测试 tmpfile
    local f = io.tmpfile()
    if f == nil then return 0 end
    if io.type(f) ~= "file" then return 0 end

    -- 写入并读取
    f:write("tmp data")
    f:seek("set", 0)
    local content = f:read("*a")
    if content ~= "tmp data" then return 0 end

    f:close()
    if io.type(f) ~= "closed file" then return 0 end

    return 6000
end
