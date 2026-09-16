-- goto 跳过 local function
function test_goto_skip_local_function()
    goto my_label
    local function f()
        return 1
    end
    ::my_label::
    return 0
end
