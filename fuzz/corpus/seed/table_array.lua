local t = {1, 2, 3, 4, 5}

function array_ops()
    t[2] = 10
    local x = t[1]
    return x + t[2] + #t
end
