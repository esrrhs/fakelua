-- goto 在 if/else 分支中
function test_goto_in_if_else()
    local x = 0
    if x == 0 then
        goto skip
        x = 99
    else
        x = 50
    end
    ::skip::
    return x
end
