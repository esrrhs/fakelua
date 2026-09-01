package "FileSetvbufCases"

-- 测试 file:setvbuf no 模式
function test_file_setvbuf_no()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    local result = f:setvbuf("no")
    if not result then return 0 end
    f:close()
    os.remove("test_io_tmp.txt")
    return 1
end

-- 测试 file:setvbuf full 模式
function test_file_setvbuf_full()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    local result = f:setvbuf("full", 8192)
    if not result then return 0 end
    f:close()
    os.remove("test_io_tmp.txt")
    return 1
end

-- 测试 file:setvbuf line 模式
function test_file_setvbuf_line()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    local result = f:setvbuf("line")
    if not result then return 0 end
    f:close()
    os.remove("test_io_tmp.txt")
    return 1
end
