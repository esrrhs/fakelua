-- 文件级 function 声明（全局函数）同样是函数绑定，再赋值编译期报错。
function handler()
    return 1
end

function init()
    handler = nil
    return 0
end
