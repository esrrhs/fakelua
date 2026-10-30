-- 文件级表绑定禁止整槽再赋，但字段赋值 t.a = 1 编译期与运行期均允许。
local t = { a = 0 }

function touch()
    t.a = 1
    return t.a
end
