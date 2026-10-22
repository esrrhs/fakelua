function f()
    local x <const> = 1
    local g = function()
        x = 2
    end
    return g
end
