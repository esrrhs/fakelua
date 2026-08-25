function test_io_read_write()
    -- 测试写入多种类型
    local f = io.open("test_io_rw.txt", "w")
    if f == nil then return 0 end

    -- 标准 Lua 的 file:write 只接受字符串或数字，布尔值应报错，这里不再测试写入 true
    f:write("abc")
    f:write(123)
    f:write(45.67)
    f:close()

    -- 读取验证
    local f2 = io.open("test_io_rw.txt", "r")
    if f2 == nil then return 0 end

    local content = f2:read("*a")
    -- 内容应该是 "abc12345.67"
    if content ~= "abc12345.67" then
        -- 浮点格式可能不同，检查前缀
        if string.sub(content, 1, 10) ~= "abc12345.6" then return 0 end
    end

    f2:close()

    -- 测试逐行读取
    local f3 = io.open("test_io_rw.txt", "w")
    f3:write("line1\n")
    f3:write("line2\n")
    f3:write("line3\n")
    f3:close()

    local f4 = io.open("test_io_rw.txt", "r")
    local l1 = f4:read("*l")
    local l2 = f4:read("*l")
    local l3 = f4:read("*l")
    local l4 = f4:read("*l")  -- 应该返回 nil
    f4:close()

    if l1 ~= "line1" then return 0 end
    if l2 ~= "line2" then return 0 end
    if l3 ~= "line3" then return 0 end
    if l4 ~= nil then return 0 end

    -- *n 读 inf/nan 不得对 double→int64 做 UB
    local fn = io.open("test_io_rw.txt", "w")
    fn:write("inf\n")
    fn:close()
    local fn2 = io.open("test_io_rw.txt", "r")
    local infv = fn2:read("*n")
    fn2:close()
    if infv ~= infv then return 0 end
    if infv ~= math.huge then return 0 end

    -- 清理
    os.remove("test_io_rw.txt")

    return 6000
end
