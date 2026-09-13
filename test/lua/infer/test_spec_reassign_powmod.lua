-- Fast modular exponentiation.
-- Parameters base, exp, mod are all math params; base and exp are reassigned
-- inside the while loop, exercising the "reassigned params are still specialized" feature.
function test(base, exp, mod)
    local result = 1
    base = base % mod
    while exp > 0 do
        if exp % 2 == 1 then
            result = (result * base) % mod
        end
        base = (base * base) % mod
        exp = exp // 2
    end
    return result
end
