-- 覆盖 AllPathsReturn 的两条边界路径：
-- line 836: AllPathsReturn(null) -- if-elseif 结构无 else 时触发
-- line 840: AllPathsReturn(empty block) -- else 分支为空时触发
-- n*2 使 n 成为数学参数，确保 AllPathsReturn 在 BuildFunctionReturnCache 路径中被调用。

-- 测试 line 836: if-elseif 所有分支均 return，但无 else（ElseBlock()==null）。
-- AllPathsReturn(body) → if/elseif 均 true → 尝试 AllPathsReturn(ElseBlock()) = AllPathsReturn(null) → line 836。
function test_no_else(n)
    if n > 10 then
        return n * 2
    elseif n > 5 then
        return n + 1
    end
end

-- 测试 line 840: else 分支为空（block->Stmts().empty()）。
-- AllPathsReturn(body) → if true → 无 elseif → AllPathsReturn(empty_else) → line 840。
function test_empty_else(n)
    if n > 10 then
        return n * 2
    else
    end
end
