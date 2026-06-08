function test(x)
    local t = { a = 1 }
    -- t.a is a table field access; exercises VarKind != kSimple path in expression compilation
    local val = t.a + x
    return val
end
