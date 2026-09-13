-- 跨函数 global 回退 + 冲突 if 分支的 soundness 测试。
--
-- 背景：TypeInferencer 的前向分析中，非 init 函数的赋值只写 local、不写 global，
-- 以匹配 CGen 特化编译的行为。但若全局变量在顶层（init）已获得 spec、随后在非 init
-- 函数里被冲突 if 分支覆盖，原版 CGen 普通编译会把 global 也冲掉（汇合后擦除），
-- 使后续读取降级为安全的动态哈希路径；前向分析必须复刻这一降级，否则会生成
-- FL_SPEC 裸指针偏移读到错误结构体内存。
--
-- 语义：
--   顶层：g = { a = 100 }   → global[g] 持有 spec_a
--   foo(cond):
--     if cond > 0  then g = { x = 1 }   (spec_x)
--     else             g = { y = 2 }   (spec_y)
--     return g.a
--   汇合后 g 的真实形状可能是 {x=1} 或 {y=2}，均不含字段 a，
--   故 g.a 必须走 FlGetTableStrId 动态路径（读到 nil），不能走 FL_SPEC。
-- 预期：cond=1 → g 实际为 {x=1}，g.a=nil → 返回 nil(0)；cond=0 → 同。
local g = { a = 100 }

function foo(cond)
    if cond > 0 then
        g = { x = 1 }
    else
        g = { y = 2 }
    end
    local v = g.a
    if v == nil then
        v = 0
    end
    return v
end
