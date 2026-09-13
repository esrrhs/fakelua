-- Unary minus on an integer variable → T_INT, declared as int64_t.
-- n = 5, x = -n → x = -5
function test()
    local n = 5
    local x = -n
    return x
end
