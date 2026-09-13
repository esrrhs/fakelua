package "IoTmpfileCases"

-- 测试 io.tmpfile
function test_io_tmpfile()
    local f = io.tmpfile()
    if not f then return 0 end
    f:write("tmp data")
    f:seek("set")
    local data = f:read("*a")
    f:close()
    if data ~= "tmp data" then return 0 end
    return 1
end
