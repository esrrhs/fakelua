function make_counter()
    local n = 0
    return function()
        n = n + 1
        return n
    end
end
local c = make_counter()
