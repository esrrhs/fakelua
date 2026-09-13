-- Runtime exception path: a helper that always returns a non-numeric string is
-- assigned to a typed-int specialisation parameter via the CVar fallback path.
-- The generated guard must fire FakeluaThrowError so that a std::exception is
-- thrown with the message "attempt to assign non-numeric value to typed int
-- variable".
--
-- n is a math param (n + 1 triggers int specialisation → test_0(int64_t n)).
-- Inside the int spec, `n = make_string_val()` goes through the CVar fallback
-- and the CVar has type VAR_STRING.  The if/else chain:
--   if (tmp.type_ == VAR_INT) { ... }
--   else if (tmp.type_ == VAR_FLOAT) { ... }
--   else { FakeluaThrowError(...); }   ← this branch fires
--
-- Calling test(5) must throw; the return statement is never reached.

function make_string_val()
    return "hello"
end

function test(n)
    local x = n + 1
    n = make_string_val()
    return n + x
end
