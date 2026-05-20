-- if-elseif 结构中 elseif 块不包含 return。
-- AllPathsReturn 检查 elseif 块时发现无 return，立即返回 false。
-- n * 2 使 n 成为数学参数，生成 int/double 特化版本。
-- test(15) == 30 (n > 10 path), test(3) == 3 (else path).
function test(n)
    if n > 10 then
        return n * 2
    elseif n > 5 then
        local x = n + 1  -- no return in this branch
    else
        return n
    end
end
