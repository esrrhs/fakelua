-- 捕获 upvalue 的尾递归，以及无捕获函数尾调到带捕获的函数（trampoline）
function test()
    local acc = 0
    local function rec(n)
        if n <= 0 then
            return acc
        end
        acc = acc + n
        return rec(n - 1)
    end
    local x = 7
    local function boxed(n)
        if n <= 0 then
            return x
        end
        return boxed(n - 1)
    end
    local function start(n)
        return boxed(n)
    end
    return rec(10) + start(5)
end
