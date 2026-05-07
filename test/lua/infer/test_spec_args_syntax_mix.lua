-- Function-call args syntax coverage for math-param specialisation analysis.
-- src/compile/README.md grammar allows args as:
--   (explist) | tableconstructor | LiteralString
-- Keep tableconstructor/string calls in dead branches and use a normal
-- numeric call on the live path, so behaviour is deterministic.

function callee(n)
    return n + 1
end

function test(n)
    if false then
        callee { n }
        callee "unused"
    end
    return callee(n)
end
