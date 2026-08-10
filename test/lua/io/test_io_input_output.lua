function test_io_input_output()
    -- 1. 写入临时文件并验证
    local tmpfile = io.tmpfile()
    tmpfile:write("hello world")
    tmpfile:seek("set", 0)
    local content = tmpfile:read("*a")
    tmpfile:close()
    if content ~= "hello world" then return 1 end

    -- 2. io.type 验证文件类型
    local f = io.tmpfile()
    if io.type(f) ~= "file" then return 2 end
    f:close()
    if io.type(f) ~= "closed file" then return 3 end

    return 5000
end
