-- Cross-scope mutation: state is assigned "error" (no `local`!) inside a
-- nested if block deep inside a for loop.  The assignment modifies the outer
-- state variable, degrading it from T_INT to T_DYNAMIC.  The compiler must
-- track this cross-scope update so that the declaration of state uses CVar.
function test()
    local state = 1          -- outer state, initially T_INT
    for i = 1, 10 do
        if i == 5 then
            state = "error"  -- no local! mutates the outer state
        end
    end
    return state             -- "error" after the loop
end
