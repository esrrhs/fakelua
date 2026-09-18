function test_sub_no_space()
    local x = 5
    if x-1 ~= 4 then return 0 end
    if 1-1 ~= 0 then return 0 end
    if x -1 ~= 4 then return 0 end
    if 10-3 ~= 7 then return 0 end
    if -2-3 ~= -5 then return 0 end
    return 1
end

function test_unary_minus_still_works()
    local a = -10
    if a ~= -10 then return 0 end
    if -0xff ~= -255 then return 0 end
    return 1
end
