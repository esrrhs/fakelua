function test_goto_backward_loop()
    local i = 0
    ::loop::
    i = i + 1
    if i < 5 then
        goto loop
    end
    return i
end
