package "IoFlushCases"

-- 测试 io.flush
function test_io_flush()
    local ok = io.flush()
    if not ok then return 0 end
    return 1
end
