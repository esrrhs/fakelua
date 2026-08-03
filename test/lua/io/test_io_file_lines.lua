function test_io_file_lines()
    -- 写入测试文件
    local f = io.open("test_file_lines_tmp.txt", "w")
    if f == nil then return 0 end
    f:write("alpha\n")
    f:write("beta\n")
    f:write("gamma\n")
    f:write("delta")  -- 最后一行无换行
    f:close()

    -- 重新打开用于读取
    local f2 = io.open("test_file_lines_tmp.txt", "r")
    if f2 == nil then return 0 end

    -- 验证 io.type
    if io.type(f2) ~= "file" then return 0 end

    -- 使用 file:lines() 迭代
    local lines = {}
    for line in f2:lines() do
        table.insert(lines, line)
    end

    -- 验证读取到 4 行
    if #lines ~= 4 then return 0 end
    if lines[1] ~= "alpha" then return 0 end
    if lines[2] ~= "beta" then return 0 end
    if lines[3] ~= "gamma" then return 0 end
    if lines[4] ~= "delta" then return 0 end

    f2:close()

    -- 验证直接使用 io.lines(filename) 迭代器
    local lines2 = {}
    for line in io.lines("test_file_lines_tmp.txt") do
        table.insert(lines2, line)
    end
    if #lines2 ~= 4 or lines2[2] ~= "beta" then return 0 end

    -- 验证关闭后类型
    if io.type(f2) ~= "closed file" then return 0 end

    -- 清理临时文件
    os.remove("test_file_lines_tmp.txt")

    return 5000
end
