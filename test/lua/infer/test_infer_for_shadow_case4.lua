-- case4:
-- inner loop variable is assigned a string; outer a should stay numeric.
function test()
    local a = 2
    for a = 1, 10 do
        a = "test"
    end
    a = a + 1
    return a
end
