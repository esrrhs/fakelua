-- if-else 两分支构造不同 key 的 table 赋给同一变量。
--   then 分支: a = {a=1}  (spec shape {a})
--   else 分支: a = {b=2}  (spec shape {b})
-- 汇合后 a 的静态类型无法用单一裸指针偏移安全访问，必须通过合并布局
-- {a?, b?} 并走 FL_SPEC 偏移读，缺失字段在 emit 时显式 nil 初始化。
-- 预期：b>0 → 1+0=1；b<=0 → 0+2=2。
function test(b)
    local a;
    if b > 0 then
        a = {a=1}
    else
        a = {b=2}
    end
    a.a = a.a or 0;
    a.b = a.b or 0;
    return a.a + a.b
end
