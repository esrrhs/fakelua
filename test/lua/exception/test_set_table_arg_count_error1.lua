-- FAKELUA_SET_TABLE with only 2 args (missing value) should error
function test(t, k)
    FAKELUA_SET_TABLE(t, k)
end
