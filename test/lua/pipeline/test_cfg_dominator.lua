-- 支配关系测试: if-else 后汇合
-- dom(merge) = {entry, cond, merge} (不应包含 then 或 else)
function test()
    local a = 1
    if a > 0 then
        a = 10
    else
        a = 20
    end
    local b = a
    return b
end
