function sum(...)
    local t = {...}
    local s = 0
    for i = 1, #t do
        s = s + t[i]
    end
    return s
end
