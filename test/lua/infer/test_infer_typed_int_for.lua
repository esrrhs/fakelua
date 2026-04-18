-- Type inference success: all ints, constant bounds.
-- sum(1..10) = 55; i and sum must stay T_INT throughout.
function test()
    local sum = 0
    for i = 1, 10 do
        sum = sum + i
    end
    return sum
end
