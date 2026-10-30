-- 修改 init 中创建的文件级表字段应抛出运行时异常（创建时 CONST_FLAG）。
local t = { a = 1, b = 2 }

function test_modify_const()
    t.a = 100 -- 应该抛出 "attempt to modify a const table"
    return 0
end
