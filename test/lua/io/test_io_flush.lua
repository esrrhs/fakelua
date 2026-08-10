function test_io_flush()
    -- 1. flush stdout（不应崩溃）
    io.flush()

    -- 2. flush 打开的文件
    local tmpfile = io.tmpfile()
    tmpfile:write("test data")
    tmpfile:flush()
    tmpfile:close()

    -- 3. 多次 flush
    io.flush()
    io.flush()

    return 5000
end
