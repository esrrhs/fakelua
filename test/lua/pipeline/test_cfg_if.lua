-- CFG 测试: if-else 分支合并
-- 期望 CFG: entry + cond + then + else + merge + exit
function test()
    local a = 1
    if a > 0 then
        a = 2
    else
        a = 3
    end
    return a
end
