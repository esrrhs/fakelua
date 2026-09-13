function test_io_popen()
    -- 用 popen 执行 echo 命令读取输出（Linux/macOS 通用）
    local f = io.popen("echo hello_popen", "r")
    if f == nil then return 0 end

    -- 验证类型
    if io.type(f) ~= "file" then return 0 end

    -- 读取输出
    local content = f:read("*a")
    if content == nil then return 0 end
    -- echo 会加换行
    content = string.gsub(content, "\n$", "")
    if content ~= "hello_popen" then return 0 end

    -- 关闭（内部使用 pclose）
    local ok = f:close()
    if ok ~= true then return 0 end

    -- 关闭后类型应为 "closed file"
    if io.type(f) ~= "closed file" then return 0 end

    -- popen 不存在的命令应返回 nil, err
    local f2, err = io.popen("this_command_does_not_exist_xyz_123", "r")
    -- 注意：某些 shell 可能仍返回一个 pipe，但执行失败
    -- 这里只验证接口行为：要么返回 nil+err，要么返回能关闭的文件
    if f2 == nil then
        if type(err) ~= "string" then return 0 end
    else
        f2:close()
    end

    return 5000
end
