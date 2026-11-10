-- 文件级命名互递归尾调用（CALLNAME/TAILNAME）
function even(n)
    if n == 0 then
        return 1
    end
    return odd(n - 1)
end

function odd(n)
    if n == 0 then
        return 0
    end
    return even(n - 1)
end

function test()
    return even(10) + even(11) + odd(10) + odd(11)
end
