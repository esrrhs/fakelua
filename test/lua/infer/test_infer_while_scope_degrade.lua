-- InferBlock(new_scope=true) in a While body degrades an outer T_INT variable.
-- x is defined as T_INT (literal 5) in the function scope.
-- The while-body block is processed with new_scope=true.  Inside the body,
-- x = n (n is T_DYNAMIC) calls env_.Update, which finds x in the outer scope
-- and merges MergeType(T_INT, T_DYNAMIC) = T_DYNAMIC there.
-- test(0) : loop runs n=0 (x=0,n=1) then n=1 (x=1,n=2), exit → x=1
-- test(3) : 3 < 2 is false, loop body never executes               → x=5
function test(n)
    local x = 5
    while n < 2 do
        x = n
        n = n + 1
    end
    return x
end
