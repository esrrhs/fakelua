package "FileLines"

-- 测试 file:lines 迭代器
function test_file_lines()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("line1\nline2\nline3")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    local lines = f:lines()
    local count = 0
    for line in lines do
        count = count + 1
    end
    f:close()
    os.remove("test_io_tmp.txt")
    if count ~= 3 then return 0 end
    return 1
end
