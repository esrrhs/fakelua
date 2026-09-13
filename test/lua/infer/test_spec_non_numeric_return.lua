-- f uses n arithmetically (n + 1 creates arithmetic improvement), so n is a
-- math param and f gets a specialised version f_0 for int inputs.
-- However, f always returns the string "hello", NOT a numeric result.
-- InferArgTypeForSpec for f(n) must return T_DYNAMIC (not T_INT), because the
-- fixed-point return-type inference recognises that f_0 returns a string.
-- test calls f(n) and returns its result: the caller must treat f(n) as CVar.
local function f(n)
    local _ = n + 1
    return "hello"
end

function test(n)
    return f(n)
end
