-- 修改全局 const 表应该抛出运行时异常
local t = {a = 1, b = 2}

function test_modify_const()
    -- 使用 pcall 捕获修改 const 表的异常
    local ok, err = pcall(function()
        t.a = 100
    end)
    if ok then
        return 0  -- 不应该成功
    end
    -- 验证异常消息包含预期内容
    if type(err) == "string" and string.find(err, "attempt to modify a const table") then
        return 5000  -- 成功捕获到预期错误
    end
    return 0  -- 错误消息不匹配
end
