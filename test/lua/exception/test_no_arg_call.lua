-- compute 是数学函数（return n*2），test 也是数学函数（return n*2）。
-- test 体内调用 compute()（无参数），参数数量不匹配，编译时应抛出异常。
function compute(n)
    return n * 2
end

function test(n)
    compute()
    return n * 2
end
