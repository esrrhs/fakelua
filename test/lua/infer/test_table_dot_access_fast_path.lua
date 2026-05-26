-- Table dot access uses FlGetTableStrId fast path.
-- t.name directly emits FlGetTableStrId instead of boxing key as CVar string.
-- test() returns t.a + t.b == 10 + 20 == 30.
function test()
    local t = {a = 10, b = 20, c = 30}
    return t.a + t.b
end
