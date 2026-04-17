-- InferBlock(new_scope=true) in an If body degrades an outer T_INT variable.
-- x is defined as T_INT (literal 5) in the function scope.
-- The if-body block is processed with new_scope=true.  Inside the body,
-- the assignment x = n (n is T_DYNAMIC, a parameter) calls env_.Update,
-- which walks scopes outward, finds x in the outer scope and merges
-- MergeType(T_INT, T_DYNAMIC) = T_DYNAMIC there.
-- The post-pass then updates x's LocalVar initialiser to T_DYNAMIC.
-- test(2) = 2   (condition true, x overwritten)
-- test(0) = 5   (condition false, x unchanged)
function test(n)
    local x = 5
    if n > 0 then
        x = n
    end
    return x
end
