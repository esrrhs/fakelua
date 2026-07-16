function test()
    local x = "temp" -- x is declared as CVar
    x = 1            -- x is assigned T_INT
    local y = x + 1  -- y is x + 1, x is T_INT here
    return y
end
