-- CFG 测试: 基本顺序语句
-- 期望 CFG: entry + exit = 2 blocks (顶层代码无分支)
function test()
    local a = 1
    local b = 2
    local c = a + b
    return c
end
