-- if-else 两分支构造同 key、不同值类型的 table 赋给同一变量。
--   then 分支: a = { x = 10 }       (x 为 int)
--   else 分支: a = { x = 2.5 }       (x 为 float)
-- 汇合后 a.x 的静态类型由 T_INT 提升为 T_FLOAT，但 table 特化布局相同（都是 {x}），
-- 故仍可走 FL_SPEC 偏移读；值类型差异由 CGen 在 emit 时按推断类型处理。
-- 预期：cond=1 → 10.0；cond=0 → 2.5。
function test_if_else_same_key_diff_type(cond)
    local a
    if cond > 0 then
        a = { x = 10 }
    else
        a = { x = 2.5 }
    end
    return a.x
end
