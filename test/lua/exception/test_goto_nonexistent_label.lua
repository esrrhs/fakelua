-- goto 跳转到不存在的 label
function test_goto_nonexistent_label()
    goto nonexistent
    return 1
end
