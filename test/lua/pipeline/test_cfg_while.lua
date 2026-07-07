-- CFG 测试: while 循环
-- 期望 CFG: entry + header + body + back_edge + exit
function test()
    local i = 0
    while i < 10 do
        i = i + 1
    end
    return i
end
