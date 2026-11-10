-- 尾调用必须保留全部返回值；return (f()) 不是尾调用，这里只断言尾路径的多返回值。
function multi()
    return 1, 2
end

function tail_multi()
    return multi()
end

function test()
    local a, b = tail_multi()
    if b == nil then
        return -1
    end
    return a + b
end
