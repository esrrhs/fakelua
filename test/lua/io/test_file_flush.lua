package "FileFlush"

-- 测试 file:flush
function test_file_flush()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("test")
    local ok = f:flush()
    if not ok then return 0 end
    f:close()
    os.remove("test_io_tmp.txt")
    return 1
end
