-- continue 跳过局部变量声明（应报错，与 goto 语义一致）
function test()
    for i = 1, 10 do
        continue
        local x = 1
    end
    return 1
end
