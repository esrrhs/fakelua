package "IoLines"

-- 测试 io.lines 带文件名
function test_io_lines_filename()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("A\nB\nC")
    f:close()

    local count = 0
    for line in io.lines("test_io_tmp.txt") do
        count = count + 1
    end
    os.remove("test_io_tmp.txt")
    if count ~= 3 then return 0 end
    return 1
end

-- 测试 io.lines 不存在的文件
function test_io_lines_no_file()
    local iter = io.lines("nonexistent_file_xyz.txt")
    if iter ~= nil then return 0 end
    return 1
end
