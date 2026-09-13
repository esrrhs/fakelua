-- Math param n: n + #s should use the native arithmetic fast path because
-- InferArgTypeForSpec now returns T_INT for the # (NUMBER_SIGN) operator,
-- and CompileNumericExp handles it via FlLenInt.
-- For n=10 and s="hello" (length 5): test(10) == 15.
function test(n)
    local s = "hello"
    return n + #s
end
