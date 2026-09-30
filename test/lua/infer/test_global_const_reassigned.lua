-- 有初值的文件级数值 local 是常量。函数里再赋值必须在编译期报错，
-- 并带上 Lua 文件位置，而不是等生成的 C 代码因为 const 赋值才失败。
-- local x = func() 不在此列：声明会降成 nil，只由 __fakelua_init 赋值一次。
local next_bot_id = 0

function init()
    next_bot_id = 800000
    return next_bot_id
end
