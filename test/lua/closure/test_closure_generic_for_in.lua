-- 工厂生成闭包迭代器
function range(start_val, end_val)
    local curr = start_val - 1
    return function()
        curr = curr + 1
        if curr <= end_val then
            return curr, curr * 2
        end
    end
end

function test()
    local sum_val = 0
    local sum_double = 0

    -- 验证自定义闭包生成器 iterator for-in 循环
    for i, d in range(1, 5) do
        sum_val = sum_val + i
        sum_double = sum_double + d
    end

    return sum_val * 10 + sum_double
end
