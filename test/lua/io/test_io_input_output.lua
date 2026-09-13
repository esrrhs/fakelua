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

    -- 3. 标准流包装是 file
    if io.type(io.stdin()) ~= "file" then return 4 end
    if io.type(io.stdout()) ~= "file" then return 5 end
    if io.type(io.stderr()) ~= "file" then return 6 end
    if io.type(io.input()) ~= "file" then return 7 end
    if io.type(io.output()) ~= "file" then return 8 end

    return 5000
end
