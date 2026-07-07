-- CFG 测试: 嵌套 if-else
-- 期望 CFG: 多层分支 + 汇合
function test()
    local a = 1
    if a > 0 then
        if a > 10 then
            a = 100
        else
            a = 50
        end
    else
        a = 0
    end
    return a
end
