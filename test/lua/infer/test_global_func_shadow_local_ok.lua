-- 函数内同名局部变量是独立槽位，赋值合法——不得因文件级同名函数绑定而误报。
local function fn()
    return 1
end

function init()
    local fn = 5
    fn = 6
    return fn
end
