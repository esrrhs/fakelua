function test_file_setvbuf()
    -- 写入测试文件
    local f = io.open("test_setvbuf_tmp.txt", "w")
    if f == nil then return 0 end

    -- 测试 setvbuf 成功时返回 file 对象
    local result = f:setvbuf("full")
    if io.type(result) ~= "file" then return 0 end

    -- 测试行缓冲
    local result2 = f:setvbuf("line")
    if io.type(result2) ~= "file" then return 0 end

    -- 测试无缓冲
    local result3 = f:setvbuf("no")
    if io.type(result3) ~= "file" then return 0 end

    -- 测试带缓冲区大小
    local result4 = f:setvbuf("full", 4096)
    if io.type(result4) ~= "file" then return 0 end

    -- 写入数据验证功能正常
    f:write("hello setvbuf\n")
    f:close()

    -- 验证文件内容
    local f2 = io.open("test_setvbuf_tmp.txt", "r")
    if f2 == nil then return 0 end
    local content = f2:read("*a")
    if content ~= "hello setvbuf\n" then return 0 end
    f2:close()

    -- 清理
    os.remove("test_setvbuf_tmp.txt")

    return 5000
end
