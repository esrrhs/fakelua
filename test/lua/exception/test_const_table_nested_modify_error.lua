-- init 中创建的表整棵树只读：嵌套字段 t.inner.x = 1 运行期也应抛 const table。
local t = { inner = { x = 0 } }

function test_modify_nested()
    t.inner.x = 1
    return 0
end
