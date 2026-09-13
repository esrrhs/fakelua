local MY_CONST = 42

function test(x)
    -- MY_CONST is a global constant of type T_INT; exercises the T_INT const branch
    local val = x + MY_CONST
    return val
end
