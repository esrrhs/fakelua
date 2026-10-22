function f()
    local x <const> = 1
    local function g()
        x = 2
    end
    return g
end
