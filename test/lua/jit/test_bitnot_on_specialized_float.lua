function test(xf)
    -- xf is specialized float; ~ on float exercises the degrade/cast path
    local dummy = xf + 1.0
    local val = ~xf
    return val
end
