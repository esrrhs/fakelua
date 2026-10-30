-- 文件级表绑定禁止整槽再赋，但字段赋值 x.a = 1 编译期允许
-- （预处理为 FAKELUA_SET_TABLE；运行期仍走既有 CONST_FLAG / nativeobj 检查）。
local t = { a = 0 }

function touch()
    t.a = 1
    return 0
end
