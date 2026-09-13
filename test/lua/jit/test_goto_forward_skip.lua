function test_goto_forward_skip()
    local x = 100
    goto skip
    x = 999
    ::skip::
    return x
end
