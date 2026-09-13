-- Bug 1 regression test: post-processing must not pollute trial inference snapshots.
-- n * 2 in the local init gets degraded (y is later reassigned to string).
-- With the fix: trial inference skips post-processing, arithmetic nodes
-- keep their true types, and n is correctly identified as a math param.
-- The function should produce specialized versions.
-- test(5) == 10, test(2.5) == 5.0
function test(n)
    local y = n * 2
    y = "override"
    return n * 2
end
