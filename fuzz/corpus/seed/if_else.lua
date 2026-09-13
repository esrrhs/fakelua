local threshold = 3

function classify(x)
    if x > threshold then
        return 10
    elseif x > 0 then
        return 1
    else
        return 0
    end
end
