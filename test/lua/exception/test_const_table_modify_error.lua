-- 文件级表绑定禁止整槽再赋，但表字段可以修改（不再打 CONST_FLAG）。
local t = { a = 1, b = 2 }

function test_modify_const()
    t.a = 100
    return t.a
end
