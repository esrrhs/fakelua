function test_io_boundary()
    -- 1. io.type: 非文件对象返回 nil
    if io.type(nil) ~= nil then return 1 end
    if io.type("hello") ~= nil then return 2 end

    -- 2. io.type: 已关闭文件返回 "closed file"
    local f = io.tmpfile()
    f:close()
    if io.type(f) ~= "closed file" then return 3 end

    -- 3. file:read: 读到文件末尾返回 nil 或空串（EOF）
    local f4 = io.tmpfile()
    f4:write("x")
    f4:seek("set", 0)
    local d = f4:read("*a")
    if d ~= "x" then f4:close(); return 4 end
    local e = f4:read("*a")
    if e ~= nil and e ~= "" then f4:close(); return 5 end
    f4:close()

    -- 4. file:write 写入成功返回非 nil（写入后文件位置前进）
    local f5 = io.tmpfile()
    local before = f5:seek("cur", 0)
    f5:write("abc")
    local after = f5:seek("cur", 0)
    if after ~= before + 3 then f5:close(); return 6 end
    f5:close()

    return 5000
end
