-- 递归函数，触发栈溢出保护
function recurse(n)
    return recurse(n + 1)
end
