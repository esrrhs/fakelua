package "FileWrite"

-- 测试 file:write 多参数
function test_file_write_multi()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("Hello", " ", "World")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    local content = f:read("*a")
    f:close()
    os.remove("test_io_tmp.txt")
    if content ~= "Hello World" then return 0 end
    return 1
end

-- 测试 file:write 返回 self
function test_file_write_returns_self()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    local result = f:write("test")
    f:close()
    os.remove("test_io_tmp.txt")
    if result == nil then return 0 end
    if type(result) ~= "userdata" and type(result) ~= "table" then return 0 end
    return 1
end
