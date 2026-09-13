-- return 之后再跟一个 do...end 块同样非法。
function test_return_before_nested_block()
    return 1
    do
        local x = 2
    end
end
