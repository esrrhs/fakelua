-- if-else 分支 table shape 冲突的 soundness 测试。
-- 两个分支构造不同 shape 的 table 赋给同一变量 a：
--   then 分支: a = { x = 10 }  (spec shape X)
--   else 分支: a = { y = 11 }  (spec shape Y)
-- 汇合后 a 的静态类型无法统一为单一 spec 结构体，必须降级为 dynamic，
-- 让 a.x / a.y 走 FlGetTableStrId 动态路径，而非 FL_SPEC 裸指针偏移读
-- （后者会读到错误结构体内存，导致内存不安全）。
-- 预期：cond=1 → 10+0=10；cond=0 → 0+11=11。
function test_if_else_soundness(cond)
    local a
    if cond > 0 then
        a = { x = 10 }
    else
        a = { y = 11 }
    end
    a.x = a.x or 0
    a.y = a.y or 0
    return a.x + a.y
end
