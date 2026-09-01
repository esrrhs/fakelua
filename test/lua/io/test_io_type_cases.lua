package "IoTypeCases"

-- 测试 io.type 文件对象
function test_io_type_file()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    local t = io.type(f)
    f:close()
    os.remove("test_io_tmp.txt")
    if t ~= "file" then return 0 end
    return 1
end

-- 测试 io.type 关闭的文件
function test_io_type_closed_file()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:close()
    local t = io.type(f)
    os.remove("test_io_tmp.txt")
    if t ~= "closed file" then return 0 end
    return 1
end

-- 测试 io.type 非文件
function test_io_type_not_file()
    local t = io.type("hello")
    if t ~= nil then return 0 end
    return 1
end

-- 测试 io.type nil
function test_io_type_nil()
    local t = io.type(nil)
    if t ~= nil then return 0 end
    return 1
end
