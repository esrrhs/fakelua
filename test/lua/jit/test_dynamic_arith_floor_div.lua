-- T_DYNAMIC floor division via CompileBinop CVar path.
-- a and b are stored in a table so t[1] and t[2] are T_DYNAMIC.
function test(a, b)
    local t = {a, b}
    local x = t[1]
    local y = t[2]
    return x // y
end
