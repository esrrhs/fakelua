-- func(n) = n * 2 is a math-param function.
-- outer declares a local variable INSIDE an if block (not at the top level):
--   when n > 0: local x = func(n); return x + 1
--   when n <= 0: return 0
-- Before the nested-block local-var fix, BuildLocalVarExtensions only scanned
-- top-level locals, so x was not in spec_ctx and "return x+1" evaluated to
-- T_DYNAMIC, leaving outer's specialization return type as T_DYNAMIC.
-- After the fix, BuildLocalVarExtensions recurses into the if block, adds
-- x:T_INT to spec_ctx (x is declared only once in the function), and
-- outer correctly gets an int64_t specialization return type.
-- outer(5) == 11 (5*2+1), outer(-1) == 0.
function func(n)
    return n * 2
end

function outer(n)
    if n > 0 then
        local x = func(n)
        return x + 1
    end
    return 0
end
