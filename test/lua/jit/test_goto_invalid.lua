-- goto 跳过局部变量声明，应该报错
function test_invalid()
    goto my_label
    local x = 1
    ::my_label::
    return x
end
