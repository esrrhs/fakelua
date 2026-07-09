-- SSA while 循环 φ 测试: header 块有 i = φ(入口值, 循环体更新)
function test()
    local i = 0
    while i < 10 do
        i = i + 1
    end
    return i
end
