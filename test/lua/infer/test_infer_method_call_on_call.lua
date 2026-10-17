-- Regression (issue #36): a method call whose receiver is itself a call
-- expression (`make():get(k)`) inside a function that takes parameters used to
-- crash the compiler with SIGSEGV, before the function was ever called.
--
-- Math-parameter discovery walks every FunctionCall node of a parameterised
-- function and used to assume the callee was a simple name: for `make():get(k)`
-- the callee node is a FunctionCall, so the cast to SyntaxTreeVar yielded
-- nullptr and GetName() dereferenced it.
local box = container.map()

function make()
    return box
end

-- The receiver is a call expression, and the enclosing function has a parameter.
function test(n)
    box:set("v", 42)
    local x = make():get("v")
    return x + n
end

-- Same shape, but the result flows into an arithmetic expression (numeric
-- specialization path) instead of a plain local binding.
function test_arith(n)
    box:set("v", 42)
    return make():get("v") * n + 1
end

-- A method call on a plain variable receiver must keep working unchanged.
function test_var(n)
    box:set("v", 42)
    local b = make()
    return b:get("v") + n
end
