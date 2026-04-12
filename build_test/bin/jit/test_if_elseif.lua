function test(a, b)
    if a > 4 and b > 4 then
        return 4
    elseif a > 3 or b > 3 then
        return 3
    elseif a > 2 or b > 2 then
        return 2
    elseif a > 1 or b > 1 then
        return 1
    end
    return 0
end
