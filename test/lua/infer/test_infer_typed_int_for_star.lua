-- Type inference: for-loop with i*2 in body.
-- sum = sum + i * 2 uses PLUS and STAR, both T_INT; sum(2*(1..5)) = 30.
function test()
    local sum = 0
    for i = 1, 5 do
        sum = sum + i * 2
    end
    return sum
end
