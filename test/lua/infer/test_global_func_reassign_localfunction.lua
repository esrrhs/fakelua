-- 文件级 local function 是「名字即函数」绑定：读取处直接构造闭包，没有可写槽位。
-- 再赋值必须在编译期报错——此前解释器静默丢弃写入、CGen 生成对右值赋值的非法 C。
local function handler()
    return 1
end

function init()
    handler = 5
    return 0
end
