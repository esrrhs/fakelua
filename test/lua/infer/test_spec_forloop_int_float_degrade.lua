-- n is pinned as int in all_int trial inference, so "n = 1.5" is ignored there.
-- In without_n trial inference, n is not pinned and becomes float after assignment.
-- The for-loop node type changes from T_INT (all_int) to T_FLOAT (without_n)
-- without becoming T_DYNAMIC.
-- m + 0 ensures the function is considered for specialization discovery.
function test(n, m)
    local acc = m + 0
    n = 1.5
    for i = 1, n do
    end
    return acc
end
