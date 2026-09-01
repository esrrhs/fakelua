package "FileRead"

-- 测试 file:read *a (读取全部)
function test_file_read_all()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("Hello World")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    local content = f:read("*a")
    f:close()
    os.remove("test_io_tmp.txt")
    if content ~= "Hello World" then return 0 end
    return 1
end

-- 测试 file:read *l (读取一行)
function test_file_read_line()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("line1\nline2\nline3")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    local l1 = f:read("*l")
    local l2 = f:read("*l")
    f:close()
    os.remove("test_io_tmp.txt")
    if l1 ~= "line1" then return 0 end
    if l2 ~= "line2" then return 0 end
    return 1
end

-- 测试 file:read *L (读取一行保留换行)
function test_file_read_line_keep()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("line1\nline2")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    local l1 = f:read("*L")
    f:close()
    os.remove("test_io_tmp.txt")
    if l1 ~= "line1\n" then return 0 end
    return 1
end

-- 测试 file:read *n (读取数字)
function test_file_read_number()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("12345")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    local n = f:read("*n")
    f:close()
    os.remove("test_io_tmp.txt")
    if n ~= 12345 then return 0 end
    return 1
end

-- 测试 file:read 数字字节
function test_file_read_bytes()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("Hello World")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    local s = f:read(5)
    f:close()
    os.remove("test_io_tmp.txt")
    if s ~= "Hello" then return 0 end
    return 1
end

-- 测试 file:read 0 字节
function test_file_read_zero()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("Hello")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    local s = f:read(0)
    f:close()
    os.remove("test_io_tmp.txt")
    if s ~= "" then return 0 end
    return 1
end

-- 测试 file:read 多格式
function test_file_read_multi_format()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("123 45.67 hello")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    local n = f:read("*n")
    local s = f:read("*l")
    f:close()
    os.remove("test_io_tmp.txt")
    if n ~= 123 then return 0 end
    return 1
end
