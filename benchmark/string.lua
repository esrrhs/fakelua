
function loop(n)
    local ret = ""
    local last = ""

    for i = 2, n do
        ret = last.."ret"..i
        last = "last"..i
    end
    return ret
end

local N = 1000000

print(loop(N))
