function test_io_type()
    -- 测试 nil
    if io.type(nil) ~= nil then return 0 end

    -- 测试字符串
    if io.type("hello") ~= nil then return 0 end

    -- 测试数字
    if io.type(123) ~= nil then return 0 end

    -- 测试文件对象
    local f = io.open("test_io_type.txt", "w")
    if f == nil then return 0 end
    if io.type(f) ~= "file" then return 0 end
    f:close()
    if io.type(f) ~= "closed file" then return 0 end

    -- 清理
    os.remove("test_io_type.txt")

    return 6000
end
