package "IoTest"

-- 测试 io.open 读取模式
function test_io_open_read()
    local f = io.open("./io/test_io_edge.lua", "r")
    if f == nil then return 0 end
    local content = f:read("*a")
    f:close()
    if type(content) ~= "string" then return 0 end
    if #content == 0 then return 0 end
    return 1
end

-- 测试 io.open 写入模式
function test_io_open_write()
    local f = io.open("test_io_edge_tmp.txt", "w")
    if f == nil then return 0 end
    f:write("hello world")
    f:close()
    -- 读取验证
    local f2 = io.open("test_io_edge_tmp.txt", "r")
    local content = f2:read("*a")
    f2:close()
    if content ~= "hello world" then return 0 end
    -- 清理
    os.remove("test_io_edge_tmp.txt")
    return 1
end

-- 测试 io.open 追加模式
function test_io_open_append()
    local f = io.open("test_io_edge_tmp.txt", "w")
    f:write("hello")
    f:close()
    local f2 = io.open("test_io_edge_tmp.txt", "a")
    f2:write(" world")
    f2:close()
    local f3 = io.open("test_io_edge_tmp.txt", "r")
    local content = f3:read("*a")
    f3:close()
    if content ~= "hello world" then return 0 end
    os.remove("test_io_edge_tmp.txt")
    return 1
end

-- 测试 io.read 各种格式
function test_io_read_formats()
    local f = io.open("test_io_edge_tmp.txt", "w")
    f:write("123 abc 45.67")
    f:close()
    local f2 = io.open("test_io_edge_tmp.txt", "r")
    local n = f2:read("*n")
    local s = f2:read("*s")
    local l = f2:read("*l")
    f2:close()
    os.remove("test_io_edge_tmp.txt")
    return 1
end

-- 测试 io.write 多参数
function test_io_write_multi()
    local f = io.open("test_io_edge_tmp.txt", "w")
    f:write("hello", " ", "world", "\n")
    f:close()
    local f2 = io.open("test_io_edge_tmp.txt", "r")
    local content = f2:read("*a")
    f2:close()
    os.remove("test_io_edge_tmp.txt")
    if content ~= "hello world\n" then return 0 end
    return 1
end

-- 测试 io.flush
function test_io_flush()
    local f = io.open("test_io_edge_tmp.txt", "w")
    f:write("test")
    f:flush()
    f:close()
    os.remove("test_io_edge_tmp.txt")
    return 1
end

-- 测试 io.type
function test_io_type()
    local f = io.open("test_io_edge_tmp.txt", "w")
    f:close()
    local f2 = io.open("test_io_edge_tmp.txt", "r")
    local t = io.type(f2)
    f2:close()
    os.remove("test_io_edge_tmp.txt")
    if t ~= "file" then return 0 end
    return 1
end

-- 测试 io.type nil
function test_io_type_nil()
    local t = io.type(nil)
    if t ~= nil then return 0 end
    return 1
end

-- 测试 file:read 行读取
function test_file_read_line()
    local f = io.open("test_io_edge_tmp.txt", "w")
    f:write("line1\nline2\nline3\n")
    f:close()
    local f2 = io.open("test_io_edge_tmp.txt", "r")
    local l1 = f2:read("*l")
    local l2 = f2:read("*l")
    local l3 = f2:read("*l")
    f2:close()
    os.remove("test_io_edge_tmp.txt")
    if l1 ~= "line1" then return 0 end
    if l2 ~= "line2" then return 0 end
    if l3 ~= "line3" then return 0 end
    return 1
end

-- 测试 file:read 全部内容
function test_file_read_all()
    local f = io.open("test_io_edge_tmp.txt", "w")
    f:write("hello world")
    f:close()
    local f2 = io.open("test_io_edge_tmp.txt", "r")
    local content = f2:read("*a")
    f2:close()
    os.remove("test_io_edge_tmp.txt")
    if content ~= "hello world" then return 0 end
    return 1
end

-- 测试 file:seek
function test_file_seek()
    local f = io.open("test_io_edge_tmp.txt", "w")
    f:write("hello world")
    f:close()
    local f2 = io.open("test_io_edge_tmp.txt", "r")
    f2:seek("set", 6)
    local content = f2:read("*a")
    f2:close()
    os.remove("test_io_edge_tmp.txt")
    if content ~= "world" then return 0 end
    return 1
end

-- 测试 file:lines 迭代器
function test_file_lines()
    local f = io.open("test_io_edge_tmp.txt", "w")
    f:write("a\nb\nc\n")
    f:close()
    local f2 = io.open("test_io_edge_tmp.txt", "r")
    local lines = {}
    for line in f2:lines() do
        table.insert(lines, line)
    end
    f2:close()
    os.remove("test_io_edge_tmp.txt")
    if #lines ~= 3 then return 0 end
    if lines[1] ~= "a" then return 0 end
    return 1
end

-- 测试 io.tmpfile
function test_io_tmpfile()
    local f = io.tmpfile()
    if f == nil then return 0 end
    f:write("temp data")
    f:seek("set")
    local content = f:read("*a")
    f:close()
    if content ~= "temp data" then return 0 end
    return 1
end

-- 测试 io.close
function test_io_close()
    local f = io.open("test_io_edge_tmp.txt", "w")
    f:write("test")
    f:close()
    os.remove("test_io_edge_tmp.txt")
    return 1
end

-- 测试 io.open 不存在的文件
function test_io_open_nonexistent()
    local f = io.open("nonexistent_file_xyz.txt", "r")
    if f ~= nil then return 0 end
    return 1
end
