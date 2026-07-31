function test_io_open_close()
    -- 测试 io.open 打开文件
    local f = io.open("test_io_tmp.txt", "w")
    if f == nil then return 0 end

    -- 测试 io.type
    if io.type(f) ~= "file" then return 0 end

    -- 写入内容
    f:write("hello")
    f:close()

    -- 测试关闭后类型
    if io.type(f) ~= "closed file" then return 0 end

    -- 重新打开读取
    local f2 = io.open("test_io_tmp.txt", "r")
    if f2 == nil then return 0 end

    local content = f2:read("*a")
    if content ~= "hello" then return 0 end

    f2:close()

    -- 清理
    os.remove("test_io_tmp.txt")

    return 6000
end
