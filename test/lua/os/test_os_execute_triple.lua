function test_os_execute_triple()
    -- 测试 os.execute(nil) —— 检查 shell 是否可用
    local ok = os.execute(nil)
    if type(ok) ~= "boolean" then return 0 end

    -- 测试 os.execute() —— 无参数
    local a, b, c = os.execute()
    if type(a) ~= "boolean" then return 0 end
    if type(b) ~= "string" then return 0 end
    if type(c) ~= "number" then return 0 end

    -- 测试成功命令
    local r1, r2, r3 = os.execute("exit 0")
    -- 在 TCC/GCC JIT 下 system() 行为一致
    -- r1 应该是 true（成功）或者 nil+"exit"+code
    if r1 ~= true and r1 ~= nil then return 0 end
    if r1 == nil then
        -- 失败时应有原因和码
        if type(r2) ~= "string" then return 0 end
        if type(r3) ~= "number" then return 0 end
    end

    return 5000
end
