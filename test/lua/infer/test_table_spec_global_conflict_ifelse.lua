-- 文件级表绑定等同 C++ const：整槽再赋 g = {...} 必须编译期报错。
-- （旧版用冲突 if 分支覆盖验证特化降级；整槽再赋已禁止，该场景不再可达。）
local g = { a = 100 }

function foo(cond)
    if cond > 0 then
        g = { x = 1 }
    else
        g = { y = 2 }
    end
    return 0
end
