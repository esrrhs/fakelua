-- goto 模拟 continue：跳过循环体剩余部分
function test_goto_continue()
    local sum = 0
    for i = 1, 10 do
        if i % 2 == 0 then
            goto continue
        end
        sum = sum + i
        ::continue::
    end
    return sum
end
