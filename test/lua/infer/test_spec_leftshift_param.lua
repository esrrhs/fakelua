-- Math param n: n << 2 should use the native arithmetic fast path in the
-- int specialization, because LEFT_SHIFT is now in kNativeArithOps and
-- InferArgTypeForSpec returns T_INT for it.  FlLShiftInt implements Lua-
-- correct clamping semantics (negative shift amounts reverse direction,
-- |shift| >= 64 returns 0).
-- For n=5: 5 << 2 = 20.
function test(n)
    return n << 2
end
