-- 修改全局 const 表应该被静默忽略
local t = {a = 1, b = 2}

function test_modify_const()
    -- 尝试修改 const 表
    t.a = 100
    -- 验证 const 保护生效：值应该保持不变
    if t.a == 1 then
        return 5000  -- 成功：const 保护生效
    end
    return 0  -- 失败：值被修改了
end
