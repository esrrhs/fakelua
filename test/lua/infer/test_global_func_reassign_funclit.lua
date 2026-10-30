-- 文件级 local 以函数字面量初始化，与 local function 一样是函数绑定，再赋值编译期报错。
local handler = function()
    return 1
end

function init()
    handler = {}
    return 0
end
