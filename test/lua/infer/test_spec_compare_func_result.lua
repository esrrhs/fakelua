-- f is a math-param function (n * 2 creates arithmetic improvement).
-- In test, x = n + 0 ensures n is also a math param.
-- In the int specialization, the condition f(n) > n compares a specialized
-- function call result with a math param: TryCompileNativeBoolExpr must use
-- InferArgTypeForSpec (not EvalType) to recognize f(n) as T_INT and emit a
-- native C comparison instead of the IsTrue dynamic path.
-- For n > 0: f(n)=2n > n is true, return x (= n).
-- For n = 0: f(0)=0 > 0 is false, return 0.
function f(n)
    return n * 2
end

function test(n)
    local x = n + 0
    if f(n) > n then
        return x
    end
    return 0
end
