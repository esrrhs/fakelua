-- FAKELUA_SET_TABLE with string arg instead of arg list should error
function test(t)
    FAKELUA_SET_TABLE "hello"
end
