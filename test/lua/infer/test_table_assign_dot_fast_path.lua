-- Table dot assignment uses FlSetTableStrId fast path via FAKELUA_SET_TABLE.
-- t.x = v is preprocessed to FAKELUA_SET_TABLE(t, "x", v) and the fast path
-- detects the string literal key.
-- test() returns t.x + t.y == 100 + 200 == 300.
function test()
    local t = {}
    t.x = 100
    t.y = 200
    return t.x + t.y
end
