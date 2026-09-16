-- 内层块里的 goto 跳过外层 local
function test_goto_nested_skip_local()
    do
        goto my_label
    end
    local x = 1
    ::my_label::
    return x
end
