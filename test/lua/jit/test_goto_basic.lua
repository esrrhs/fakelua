-- goto/label 合法用法
function test_goto_forward()
    local x = 100
    goto skip
    x = 999
    ::skip::
    return x
end

function test_goto_backward()
    local i = 0
    ::loop::
    i = i + 1
    if i < 5 then
        goto loop
    end
    return i
end

function test_label_no_goto()
    ::unused::
    return 42
end
