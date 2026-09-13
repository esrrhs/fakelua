-- Math param n: (~n) + 1 should use the native arithmetic fast path in the
-- int specialization, because InferArgTypeForSpec now recognises BITNOT of a
-- T_INT operand as T_INT.
-- For n=5: ~5 = -6, -6 + 1 = -5.
function test(n)
    return (~n) + 1
end
