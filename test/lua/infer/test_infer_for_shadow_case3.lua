-- case3:
-- outer a remains numeric; loop cursor a shadows it.
function test()
    local a = 2
    local sum = 0
    for a = 1, 10 do
        sum = sum + a
    end
    a = a + 1
    return a
end
