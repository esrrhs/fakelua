-- 文件级表绑定等同 C++ const：整槽再赋必须编译期报错。
local g = { a = 100 }

function test_global_crossfunc(cond)
    if cond > 0 then
        g = { x = 1 }
    else
        g = { y = 2 }
    end
    return 0
end
