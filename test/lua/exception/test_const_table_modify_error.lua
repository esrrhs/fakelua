-- 修改文件级 const 表字段应抛出运行时异常（CONST_FLAG）。
local t = { a = 1, b = 2 }

function test_modify_const()
    t.a = 100 -- 应该抛出 "attempt to modify a const table"
    return 0
end
