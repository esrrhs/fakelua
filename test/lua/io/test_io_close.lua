package "IoClose"

-- 测试 io.close 默认
function test_io_close_default()
    local ok = io.close()
    if not ok then return 0 end
    return 1
end

-- 测试 io.close 带文件
function test_io_close_file()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    local ok = io.close(f)
    os.remove("test_io_tmp.txt")
    if not ok then return 0 end
    return 1
end
