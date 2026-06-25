-- goto 跳过多个局部变量
function test_goto_skip_multiple_locals()
    goto my_label
    local a = 1
    local b = 2
    ::my_label::
    return a
end
