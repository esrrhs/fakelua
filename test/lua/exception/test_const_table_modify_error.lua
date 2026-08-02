-- 修改全局 const 表应该抛出运行时异常
local t = {a = 1, b = 2}

function test_modify_const()
    t.a = 100  -- 应该抛出 "attempt to modify a const table"
    return 0
end
