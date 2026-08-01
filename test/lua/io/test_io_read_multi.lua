function test_io_read_multi()
    -- 写入测试文件
    local f = io.open("test_read_multi_tmp.txt", "w")
    if f == nil then return 0 end
    f:write("42 3.14\n")
    f:write("hello world\n")
    f:write("100\n")
    f:close()

    -- 重新打开读取
    local f2 = io.open("test_read_multi_tmp.txt", "r")
    if f2 == nil then return 0 end

    -- 测试单格式读取一行
    local line1 = f2:read("*l")
    if line1 ~= "42 3.14" then return 0 end

    -- 测试多格式读取：从第二行读取文本行 + 数字
    -- 当前文件位置在 "hello world\n100\n"
    local text, num = f2:read("*l", "*n")
    if text ~= "hello world" then return 0 end
    if num ~= 100 then return 0 end

    -- 文件末尾还有一个换行，所以 read("*l") 返回空字符串
    local empty = f2:read("*l")
    if empty ~= "" then return 0 end

    -- 现在才真正 EOF，返回 nil
    local eof = f2:read("*l")
    if eof ~= nil then return 0 end

    f2:close()

    -- 测试 io.read 多格式（从文件重定向太复杂，仅测试 file:read 多格式）

    -- 清理
    os.remove("test_read_multi_tmp.txt")

    return 5000
end
