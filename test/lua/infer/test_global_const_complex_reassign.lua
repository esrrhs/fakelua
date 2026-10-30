-- 复杂初值 local x = f() 经 __fakelua_init 赋值后仍是文件级常量，整槽再赋编译期报错。
local function seed()
    return 0
end

local x = seed()

function init()
    x = 1
    return x
end
