function map(arr, fn)
    local res = {}
    for i = 1, #arr do
        res[i] = fn(arr[i])
    end
    return res
end

function test()
    local multiplier = 3
    local arr = {1, 2, 3, 4}
    local doubled = map(arr, function(val)
        return val * multiplier
    end)
    return doubled[1] + doubled[2] + doubled[3] + doubled[4]
end
