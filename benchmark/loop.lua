
function loop(n)
    local count = 0

    for i = 2, n do
        count = count + 1
    end
    return count
end

local N = 10 * 10000000

print(loop(N))
