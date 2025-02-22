function test(a, b)
    if a > 4 and b > 4 then
        return 4
    elseif a > 3 or b > 3 then
        return 3
    else
        if a > 2 then
            return 2
        end
    end
    return 0
end
