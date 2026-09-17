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
    -- 关闭当前默认输出后必须还原 stdout：同一 State 会再跑其他 JIT 后端，
    -- Lua 5.4 对已关闭的默认输出再 io.flush() 会报 standard file is closed。
    local f = io.tmpfile()
    if not f then return 0 end
    io.output(f)
    io.write("flush-default")
    io.flush()
    io.output(io.stdout())
    f:close()

    return 5000
end
