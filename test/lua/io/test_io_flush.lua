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

    -- 4. flush 走当前默认输出（io.output 设过的文件），不是写死 stdout
    local f = io.tmpfile()
    if not f then return 0 end
    io.output(f)
    io.write("flush-default")
    io.flush()
    f:close()

    return 5000
end
