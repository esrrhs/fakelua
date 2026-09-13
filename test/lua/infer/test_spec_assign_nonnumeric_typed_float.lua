-- Bug 1 fix (float path): a typed-float specialisation parameter is reassigned
-- through the CVar fallback path (make_float returns a dynamic CVar, not a
-- native double).  The generated C code must contain a runtime numeric-type
-- check so that assigning a non-numeric CVar (e.g. a string) throws an error
-- instead of silently reading a garbage double value.
--
-- n is a math param (n + 1.0 arithmetic), so test_1(double n) is generated.
-- Inside the float spec body, `n = make_float(n)` takes the CVar fallback path
-- and the fixed code emits:
--   if (tmp.type_ == VAR_FLOAT) { n = tmp.data_.f; }
--   else if (tmp.type_ == VAR_INT) { n = (double)tmp.data_.i; }
--   else { FakeluaThrowError(...); }
--
-- test(2.5) => x = 2.5+1.0 = 3.5, n = make_float(2.5) = 2.5, return 2.5 + 3.5 = 6.0.
-- test(1.0) => x = 1.0+1.0 = 2.0, n = make_float(1.0) = 1.0, return 1.0 + 2.0 = 3.0.

function make_float(x)
    return x
end

function test(n)
    local x = n + 1.0
    n = make_float(n)
    return n + x
end
