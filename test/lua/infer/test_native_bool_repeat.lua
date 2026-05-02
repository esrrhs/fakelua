-- Native bool expression in repeat..until: the until condition i > n where
-- both i (T_INT local) and n (T_INT in int specialization) are typed,
-- so TryCompileNativeBoolExpr must emit if ((i) > (n)) break; directly.
function test(n)
    local i = 1
    local s = 0
    repeat
        s = s + i
        i = i + 1
    until i > (n + 0)
    return s
end
