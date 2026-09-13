function test_io_lines_boundary()
    -- 1. 空文件
    local empty = io.tmpfile()
    local count = 0
    for line in empty:lines() do
        count = count + 1
    end
    empty:close()
    if count ~= 0 then return 0 end

    -- 2. 单行无换行符
    local f = io.tmpfile()
    f:write("single line")
    f:seek("set", 0)
    local lines = {}
    for line in f:lines() do
        table.insert(lines, line)
    end
    f:close()
    if #lines ~= 1 or lines[1] ~= "single line" then return 0 end

    -- 3. 多行文件
    local f2 = io.tmpfile()
    f2:write("line1\nline2\nline3\n")
    f2:seek("set", 0)
    local lines2 = {}
    for line in f2:lines() do
        table.insert(lines2, line)
    end
    f2:close()
    if #lines2 ~= 3 then return 0 end
    if lines2[1] ~= "line1" or lines2[2] ~= "line2" or lines2[3] ~= "line3" then return 0 end

    return 5000
end
