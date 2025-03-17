function test(a, b)
    local ret = 0
    if a > 4 and b > 4 then
        return 4
    elseif a > 3 or b > 3 then
        ret = 3
    elseif a > 2 or b > 2 then
        return 2
    elseif a > 1 or b > 1 then
        ret = 1
    end
    return ret
end
