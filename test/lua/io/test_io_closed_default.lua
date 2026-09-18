function test_io_closed_default_read()
    local f = io.open("test_io_closed_default.txt", "w")
    if not f then return 0 end
    f:write("x")
    f:close()
    f = io.open("test_io_closed_default.txt", "r")
    if not f then return 0 end
    io.input(f)
    f:close()
    os.remove("test_io_closed_default.txt")
    io.read()
    return 0
end

function test_io_closed_default_write()
    local f = io.open("test_io_closed_default_w.txt", "w")
    if not f then return 0 end
    io.output(f)
    f:close()
    os.remove("test_io_closed_default_w.txt")
    io.write("x")
    return 0
end

function test_file_read_multi_nil_count()
    local f = io.open("test_io_multi_nil.txt", "w")
    if not f then return 0 end
    f:write("123")
    f:close()
    f = io.open("test_io_multi_nil.txt", "r")
    if not f then return 0 end
    local n = select("#", f:read("*n", "*n", "*l"))
    f:close()
    os.remove("test_io_multi_nil.txt")
    if n ~= 2 then return 0 end
    return 1
end
