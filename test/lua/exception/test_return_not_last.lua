-- return 只能是所在块的最后一条语句，Lua 5.4 同样拒绝这种写法。
function test_return_not_last()
    return 1
    local x = 2
end
