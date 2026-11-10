-- 局部闭包自递归尾调用：return rec(...)
function test()
    local function rec(n, acc)
        if n <= 0 then
            return acc
        end
        return rec(n - 1, acc + n)
    end
    return rec(20, 0)
end
