function test(x)
    -- x is a specialized int; ~ exercises the unary bitnot-on-int path
    local dummy = x + 1
    local val = ~x
    return val
end
