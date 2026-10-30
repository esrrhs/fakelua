-- 文件级表：字段赋值 t.a = 1 编译期允许（FAKELUA_SET_TABLE），运行期因 CONST_FLAG 抛错。
local t = { a = 0 }

function touch()
    t.a = 1
    return t.a
end
