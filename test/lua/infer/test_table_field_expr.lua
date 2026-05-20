-- Table subscript access t[1] is T_DYNAMIC, so t[1] + n is T_DYNAMIC.
-- n is not a math param; test is not specialized.
-- test(5) == t[1] + 5 == 10 + 5 == 15.
function test(n)
    local t = {10, 20, 30}
    return t[1] + n
end
