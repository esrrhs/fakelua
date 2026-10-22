-- Shadowing a outer <const> with a non-const local must allow assignment.
function f()
    local x <const> = 1
    local function g()
        local x = 0
        x = 2
        return x
    end
    return g()
end
