-- 复杂初值（含函数调用）经 __fakelua_init 初始化后，绑定仍等同 C++ const：
-- 用户函数里整槽再赋（含 number→table 异型）必须编译期报错。
package "main"

local v = math.abs(-1) - math.abs(-1)
local stat, grp = v, v

local function build()
    -- 异型整槽再赋：应被拒绝
    stat, grp = {}, {}
    return 0
end

function init()
    return 0
end

function update()
    build()
    return 0
end

function fini()
    return 0
end
