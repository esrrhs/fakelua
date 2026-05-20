-- AllPathsReturn 边界路径：
-- test_no_else: if-elseif 所有分支均 return，但无 else（ElseBlock()==null）→ AllPathsReturn false。
-- test_empty_else: else 分支为空 block → block->Stmts().empty() → AllPathsReturn false。
-- 两个函数均有 n * 2，使 n 成为数学参数。

-- if-elseif 所有分支均 return，但无 else（ElseBlock()==null）。
function test_no_else(n)
    if n > 10 then
        return n * 2
    elseif n > 5 then
        return n + 1
    end
end

-- else 分支为空（block->Stmts().empty()）。
function test_empty_else(n)
    if n > 10 then
        return n * 2
    else
    end
end
