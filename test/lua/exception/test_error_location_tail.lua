package "TailErr"

-- 深层尾递归后出错，位置必须是最内层那条语句，而不是 C 栈溢出。
function boom(n)
    if n <= 0 then
        return 1 + nil
    end
    return boom(n - 1)
end

function call_missing(n)
    if n <= 0 then
        return does_not_exist()
    end
    return call_missing(n - 1)
end
