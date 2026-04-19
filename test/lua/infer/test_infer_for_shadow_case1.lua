-- case1:
-- outer a is degraded after loop; loop cursor a shadows outer a.
function test()
    local a = 2
    local sum = 0
    for a = 1, 10 do
        sum = sum + a
    end
    a = "test"
    return sum
end
