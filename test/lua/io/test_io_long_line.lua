function test_io_long_line()
    -- 测试超过 4096 字节的长行
    local f = io.open("test_io_long.txt", "w")
    if f == nil then return 0 end

    -- 生成一个 5000 字符的长行
    local long_line = string.rep("A", 5000)
    f:write(long_line)
    f:write("\n")
    f:write("short")
    f:close()

    -- 读取验证
    local f2 = io.open("test_io_long.txt", "r")
    if f2 == nil then return 0 end

    local line1 = f2:read("*l")
    local line2 = f2:read("*l")
    local line3 = f2:read("*l")  -- 应该返回 nil
    f2:close()

    if #line1 ~= 5000 then return 0 end
    if line1 ~= long_line then return 0 end
    if line2 ~= "short" then return 0 end
    if line3 ~= nil then return 0 end

    -- 清理
    os.remove("test_io_long.txt")

    return 6000
end
