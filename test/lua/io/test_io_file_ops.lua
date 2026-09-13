package "IoFileOps"

-- 测试 io.output/io.input
function test_io_output_input()
    return 1
end

-- 测试 file:read 读取指定字节数
function test_file_read_bytes()
    local f = io.open("test_read_bytes.txt", "w")
    if f == nil then return 0 end
    f:write("hello world")
    f:close()

    local f2 = io.open("test_read_bytes.txt", "r")
    if f2 == nil then return 0 end
    local str = f2:read(5)
    f2:close()
    os.remove("test_read_bytes.txt")
    if str ~= "hello" then return 0 end
    return 1
end

-- 测试 file:read 读取 0 字节
function test_file_read_zero_bytes()
    local f = io.open("test_read_zero.txt", "w")
    if f == nil then return 0 end
    f:write("hello")
    f:close()

    local f2 = io.open("test_read_zero.txt", "r")
    if f2 == nil then return 0 end
    local str = f2:read(0)
    f2:close()
    os.remove("test_read_zero.txt")
    if str ~= "" then return 0 end
    return 1
end

-- 测试 file:setvbuf no
function test_file_setvbuf_no()
    local f = io.open("test_setvbuf.txt", "w")
    if f == nil then return 0 end
    f:setvbuf("no")
    f:write("test")
    f:close()
    os.remove("test_setvbuf.txt")
    return 1
end

-- 测试 file:setvbuf full
function test_file_setvbuf_full()
    local f = io.open("test_setvbuf.txt", "w")
    if f == nil then return 0 end
    f:setvbuf("full", 1024)
    f:write("test")
    f:close()
    os.remove("test_setvbuf.txt")
    return 1
end

-- 测试 file:setvbuf line
function test_file_setvbuf_line()
    local f = io.open("test_setvbuf.txt", "w")
    if f == nil then return 0 end
    f:setvbuf("line", 1024)
    f:write("test")
    f:close()
    os.remove("test_setvbuf.txt")
    return 1
end

-- 测试 file:seek set
function test_file_seek_set()
    local f = io.open("test_seek.txt", "w")
    if f == nil then return 0 end
    f:write("hello world")
    f:close()

    local f2 = io.open("test_seek.txt", "r")
    if f2 == nil then return 0 end
    f2:seek("set", 6)
    local str = f2:read(5)
    f2:close()
    os.remove("test_seek.txt")
    if str ~= "world" then return 0 end
    return 1
end

-- 测试 file:seek end
function test_file_seek_end()
    local f = io.open("test_seek.txt", "w")
    if f == nil then return 0 end
    f:write("hello world")
    f:close()

    local f2 = io.open("test_seek.txt", "r")
    if f2 == nil then return 0 end
    f2:seek("end", -5)
    local str = f2:read(5)
    f2:close()
    os.remove("test_seek.txt")
    if str ~= "world" then return 0 end
    return 1
end

-- 测试 file:seek cur
function test_file_seek_cur()
    local f = io.open("test_seek.txt", "w")
    if f == nil then return 0 end
    f:write("hello world")
    f:close()

    local f2 = io.open("test_seek.txt", "r")
    if f2 == nil then return 0 end
    f2:read(5)
    f2:seek("cur", 1)
    local str = f2:read(5)
    f2:close()
    os.remove("test_seek.txt")
    if str ~= "world" then return 0 end
    return 1
end

-- 测试 file:flush
function test_file_flush()
    local f = io.open("test_flush.txt", "w")
    if f == nil then return 0 end
    f:write("test")
    f:flush()
    f:close()
    os.remove("test_flush.txt")
    return 1
end

-- 测试 file:lines
function test_file_lines()
    local f = io.open("test_lines.txt", "w")
    if f == nil then return 0 end
    f:write("a\nb\nc\n")
    f:close()

    local f2 = io.open("test_lines.txt", "r")
    if f2 == nil then return 0 end
    local lines = {}
    for line in f2:lines() do
        table.insert(lines, line)
    end
    f2:close()
    os.remove("test_lines.txt")
    if #lines ~= 3 then return 0 end
    if lines[1] ~= "a" or lines[2] ~= "b" or lines[3] ~= "c" then return 0 end
    return 1
end

-- 测试 io.popen
function test_io_popen()
    local f = io.popen("echo hello")
    if f == nil then return 0 end
    local content = f:read("*a")
    f:close()
    if content == nil or content == "" then return 0 end
    return 1
end

-- 测试 io.tmpfile
function test_io_tmpfile()
    local f = io.tmpfile()
    if f == nil then return 0 end
    f:write("tmp content")
    f:seek("set")
    local content = f:read("*a")
    f:close()
    if content ~= "tmp content" then return 0 end
    return 1
end

-- 测试 io.type
function test_io_type()
    local f = io.tmpfile()
    if f == nil then return 0 end
    local t1 = io.type(f)
    f:close()
    local t2 = io.type(f)
    if t1 ~= "file" then return 0 end
    if t2 ~= "closed file" then return 0 end
    return 1
end

-- 测试 io.close
function test_io_close()
    local f = io.tmpfile()
    if f == nil then return 0 end
    io.close(f)
    return 1
end

-- 测试 io.flush
function test_io_flush()
    io.flush()
    return 1
end
