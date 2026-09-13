function test_io_seek()
    -- 创建测试文件
    local f = io.open("test_io_seek.txt", "w")
    if f == nil then return 0 end
    f:write("0123456789ABCDEF")
    f:close()

    -- 测试 seek set
    local f2 = io.open("test_io_seek.txt", "r")
    if f2 == nil then return 0 end

    local pos = f2:seek("set", 5)
    if pos ~= 5 then return 0 end

    local content = f2:read("*a")
    if content ~= "56789ABCDEF" then return 0 end

    f2:close()

    -- 测试 seek cur
    local f3 = io.open("test_io_seek.txt", "r")
    f3:seek("set", 3)
    local pos2 = f3:seek("cur", 4)
    if pos2 ~= 7 then return 0 end
    local content2 = f3:read(1)
    if content2 ~= "7" then return 0 end
    f3:close()

    -- 测试 seek end
    local f4 = io.open("test_io_seek.txt", "r")
    local pos3 = f4:seek("end")
    if pos3 ~= 16 then return 0 end
    local content3 = f4:read("*a")
    if content3 ~= nil and content3 ~= "" then
        -- EOF 时 read("*a") 返回 nil 或空字符串都可以接受
        if content3 ~= "" then return 0 end
    end
    f4:close()

    -- 清理
    os.remove("test_io_seek.txt")

    return 6000
end
