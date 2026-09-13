-- Bug 1 fix (int path): a typed-int specialisation parameter is reassigned
-- through the CVar fallback path (make_int returns a dynamic CVar, not a
-- native int64_t).  The generated C code must contain a runtime numeric-type
-- check so that assigning a non-numeric CVar (e.g. a string) throws an error
-- instead of silently reading a garbage int64_t value.
--
-- n is a math param (n + 1 arithmetic), so test_0(int64_t n) is generated.
-- Inside the spec body, `n = make_int(n)` takes the CVar fallback path and
-- the fixed code emits:
--   if (tmp.type_ == VAR_INT) { n = tmp.data_.i; }
--   else if (tmp.type_ == VAR_FLOAT) { n = (int64_t)tmp.data_.f; }
--   else { FakeluaThrowError(...); }
--
-- test(5) => x = 5+1 = 6, n = make_int(5) = 5, return 5 + 6 = 11.
-- test(3) => x = 3+1 = 4, n = make_int(3) = 3, return 3 + 4 = 7.

function make_int(x)
    return x
end

function test(n)
    local x = n + 1
    n = make_int(n)
    return n + x
end
