-- Extra uncovered scenario:
-- inner typed local with same name must not pollute outer dynamic variable type.
function test()
    local a = "outer"
    do
        local a = 1
        a = a + 1
    end
    a = "after"
    return a
end
