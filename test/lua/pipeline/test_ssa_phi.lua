-- SSA φ 节点测试: if-else 合并处插入 φ
-- 期望: merge 块有 a_v3 = φ(a_v1, a_v2)
function test()
    local a = 1
    if a > 0 then
        a = 10
    else
        a = 20
    end
    return a  -- 使用 φ 合并后的版本
end
