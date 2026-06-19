-- C++ 直接调用变参函数的测试场景

-- 场景1: 纯变参求和
function sum(...)
    local t = {...}
    local s = 0
    for i = 1, #t do
        s = s + t[i]
    end
    return s
end

-- 场景2: 固定参数 + 变参，透传返回
function prefix_and_vararg(prefix, ...)
    return prefix, ...
end

-- 场景3: 空变参调用（... 为空）
function vararg_or_default(...)
    local t = {...}
    if #t == 0 then
        return -1
    end
    return t[1]
end
