-- case2:
-- both outer and inner `a` become dynamic assignments.
function test()
    local a = 2
    for a = 1, 10 do
        a = "test"
    end
    a = "test"
    return a
end
