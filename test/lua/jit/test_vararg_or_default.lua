function vararg_or_default(...)
    local t = {...}
    if #t == 0 then
        return -1
    end
    return t[1]
end
