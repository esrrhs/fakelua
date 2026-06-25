-- goto 跳转到另一个函数的 label（不可见）
function test_goto_cross_function_a()
    goto cross_label
    return 1
end

function test_goto_cross_function_b()
    ::cross_label::
    return 2
end
