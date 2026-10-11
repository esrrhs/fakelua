-- Math-specialized functions must deoptimize to a dynamic variant when a math
-- parameter receives a non-numeric value (string/bool/nil/table), instead of
-- throwing a hard error at the entry dispatcher. This keeps JIT (TCC/GCC)
-- behavior consistent with the INTERP backend and with Lua semantics: the
-- function body may contain a type() guard, and real arithmetic errors must
-- remain pcall-catchable rather than aborting the whole process.

-- type() guard: non-numeric argument takes the fallback branch.
function deopt_guard(x)
    if type(x) ~= "number" then
        return -1
    end
    return x + 1
end

-- Two math parameters: any non-numeric one triggers deoptimization.
function deopt_multi(a, b)
    if type(a) ~= "number" or type(b) ~= "number" then
        return 0
    end
    return a * 2 + b
end

-- Dynamic variant must be allowed to return a value of a different kind
-- (string) than the numeric specialized variants.
function deopt_kind(x)
    if type(x) == "number" then
        return x + 100
    end
    return "kind"
end

-- No guard: arithmetic on a non-numeric value must surface as a Lua error
-- (pcall catches it), not crash the process.
function deopt_arith(x)
    return x + 1
end

-- Recursive specialized function reached with a non-numeric argument.
function deopt_fact(n)
    if type(n) ~= "number" then
        return -1
    end
    if n <= 1 then
        return 1
    end
    return n * deopt_fact(n - 1)
end

-- Specialized function without explicit return at block end:
-- exercise !BlockEndsWithReturn(func_block) in __dyn emitting "return kNil;"
function deopt_noreturn(x)
    if type(x) ~= "number" then
        -- no return
    else
        x = x + 1
    end
end

local function dyn_callee()
    return "dyn_str"
end

-- Expressions with unsupported operators or dynamic calls trigger
-- ThrowSpecMiss during tentative numeric compilation and fall back cleanly.
function deopt_unsupported_ops(x)
    if type(x) ~= "number" then
        return -1
    end
    if x < 0 then
        local a = x + (~"abc")
        local b = x + (#"abc")
        local c = x + ("a" .. "b")
        local d = x + dyn_callee()
        return a + b + c + d
    end
    return x * 2
end

-- In-Lua verification entry: returns 0 only when every expectation holds.
function test_deopt()
    if deopt_guard(10) ~= 11 then
        return 1
    end
    if deopt_guard("abc") ~= -1 then
        return 2
    end
    if deopt_guard(true) ~= -1 then
        return 3
    end
    local ok4, res4 = pcall(deopt_guard)
    if not ok4 or res4 ~= -1 then
        return 4
    end
    if deopt_multi(1, 2) ~= 4 then
        return 5
    end
    if deopt_multi("x", 2) ~= 0 then
        return 6
    end
    if deopt_multi(2, "y") ~= 0 then
        return 7
    end
    local ok8, res8 = pcall(deopt_multi, 1)
    if not ok8 or res8 ~= 0 then
        return 8
    end
    if deopt_kind(5) ~= 105 then
        return 9
    end
    if deopt_kind("z") ~= "kind" then
        return 10
    end
    if deopt_fact(5) ~= 120 then
        return 11
    end
    if deopt_fact("z") ~= -1 then
        return 12
    end
    if pcall(deopt_arith, "abc") then
        return 13
    end
    if pcall(deopt_arith, {}) then
        return 14
    end
    if deopt_arith(41) ~= 42 then
        return 15
    end
    if deopt_noreturn(10) ~= nil then
        return 16
    end
    if deopt_noreturn("abc") ~= nil then
        return 17
    end
    if deopt_unsupported_ops(5) ~= 10 then
        return 18
    end
    if deopt_unsupported_ops("z") ~= -1 then
        return 19
    end
    return 0
end
