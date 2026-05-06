-- for 循环中 break 在数学参数特化函数里的正确性。
-- n 为数学参数（sum + i 是整数算术），循环在 i > 3 时提前退出。
-- test(10) == 1+2+3 = 6, test(2) == 1+2 = 3.
function test(n)
    local sum = 0
    for i = 1, n do
        if i > 3 then
            break
        end
        sum = sum + i
    end
    return sum
end
