-- goto 跳过局部变量声明（单个）
function test_goto_skip_single_local()
    goto my_label
    local x = 1
    ::my_label::
    return x
end
