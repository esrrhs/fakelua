-- goto 跳出多层嵌套循环
function test_goto_break_out()
    local result = 0
    for i = 1, 10 do
        for j = 1, 10 do
            if i * j > 20 then
                result = i * j
                goto done
            end
        end
    end
    ::done::
    return result
end
