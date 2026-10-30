-- 使用方（1627 明末项目）报告的真实触发形态：非常量初始化的文件级 local
-- 必须允许异型重绑（number -> table），三后端语义一致。
-- init 值含函数调用 → 预处理降为 nil，槽位是动态 CVar，重绑走整体替换。
package "main"

local v = math.abs(-1) - math.abs(-1)
local stat, grp = v, v

local function build()
    stat, grp = {}, {}
    return 0
end

function init()
    return 0
end

function update()
    build()
    if type(stat) ~= "table" or type(grp) ~= "table" then
        return 1
    end
    -- 同型整体重绑（table -> table）也必须继续可用
    stat, grp = { 1 }, { 2 }
    if stat[1] ~= 1 or grp[1] ~= 2 then
        return 2
    end
    return 0
end

function fini()
    return 0
end
