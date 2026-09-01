package "FileSeek"

-- 测试 file:seek set
function test_file_seek_set()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("Hello World")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    f:seek("set", 6)
    local s = f:read(5)
    f:close()
    os.remove("test_io_tmp.txt")
    if s ~= "World" then return 0 end
    return 1
end

-- 测试 file:seek end
function test_file_seek_end()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("Hello World")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    f:seek("end", -5)
    local s = f:read(5)
    f:close()
    os.remove("test_io_tmp.txt")
    if s ~= "World" then return 0 end
    return 1
end

-- 测试 file:seek cur
function test_file_seek_cur()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("Hello World")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    f:read(5)
    f:seek("cur", 1)
    local s = f:read(5)
    f:close()
    os.remove("test_io_tmp.txt")
    if s ~= "World" then return 0 end
    return 1
end

-- 测试 file:seek 无效 whence
function test_file_seek_invalid_whence()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    f:write("test")
    f:close()

    f = io.open("test_io_tmp.txt", "r")
    if not f then return 0 end
    local ok, err = pcall(function() f:seek("invalid") end)
    f:close()
    os.remove("test_io_tmp.txt")
    if ok then return 0 end
    return 1
end
