package "IoPopenCases"

-- 测试 io.popen 读模式
function test_io_popen_read()
    local f = io.popen("echo hello", "r")
    if not f then return 0 end
    local s = f:read("*a")
    f:close()
    if not string.find(s, "hello") then return 0 end
    return 1
end

-- 测试 io.popen 写模式
function test_io_popen_write()
    local f = io.popen("cat > /dev/null", "w")
    if not f then return 0 end
    f:write("test")
    f:close()
    return 1
end
