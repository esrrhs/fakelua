-- Math param n: n >> 1 should use the native arithmetic fast path in the
-- int specialization, because RIGHT_SHIFT is now in kNativeArithOps and
-- InferArgTypeForSpec returns T_INT for it.  FlRShiftInt implements Lua-
-- correct clamping semantics (negative shift amounts reverse direction,
-- |shift| >= 64 returns 0).
-- For n=16: 16 >> 1 = 8.
function test(n)
    return n >> 1
end
