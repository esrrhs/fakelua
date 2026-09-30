-- 右值是运行时值也同样禁止：常量与否不取决于赋值能不能在编译期算出来。
local map_width = 2000

function init(w)
    map_width = w
    return map_width
end
