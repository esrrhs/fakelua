-- 有初值的文件级 local 是常量（字面量或复杂初值经 __fakelua_init 初始化后均然）。
-- 函数里整槽再赋值必须在编译期报错，并带上 Lua 文件位置，
-- 而不是等生成的 C 代码因为 const 赋值才失败。
local next_bot_id = 0

function init()
    next_bot_id = 800000
    return next_bot_id
end
