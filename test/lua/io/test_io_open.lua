package "IoOpen"

-- 测试 io.open 不存在的文件
function test_io_open_no_file()
    local f, err = io.open("nonexistent_file_xyz.txt", "r")
    if f ~= nil then return 0 end
    if not err then return 0 end
    return 1
end

-- 测试 io.open 空文件名
function test_io_open_empty_name()
    local f, err = io.open("", "r")
    if f ~= nil then return 0 end
    return 1
end
