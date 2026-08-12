-- Test io error paths: bad argument types must throw
-- Covers: io.close / file:read / file:write / file:setvbuf / io.open / io.popen

function test_io_close_bad_arg()
    io.close(42)
end

function test_file_read_bad_arg()
    local f = io.tmpfile()
    f:read(true)
    f:close()
end

function test_file_write_bad_arg()
    local f = io.tmpfile()
    f:write(true)
    f:close()
end

function test_file_setvbuf_bad_mode()
    local f = io.tmpfile()
    f:setvbuf(true)
    f:close()
end

function test_io_open_bad_arg()
    io.open(true)
end

function test_io_open_bad_mode()
    io.open("/tmp/x", true)
end

function test_io_popen_bad_arg()
    io.popen(true)
end
