-- 跨函数 global 特化传递 + 后续函数读取的 soundness 测试。
--
-- 该用例验证：当全局变量 g 在 init 获得 spec、随后被某函数 foo 的冲突 if 分支
-- 覆盖后，第三个函数 bar 再读取 g.a 是否安全。
--
-- 由于 AnalyzeTableShapes 按变量做流不敏感的字段并集，g 的所有构造器（init 的 {a=100}
-- 与 foo 的 {x=1}/{y=2}）共享同一个合并布局 spec_{a,x,y}，故各赋值语句产出的 spec 名相同，
-- 汇合后保留；bar 读取 g.a 时 IsSpecField(spec_{a,x,y}, "a")=true → FL_SPEC，
-- 运行时缺失字段 a 已被 nil 初始化 → 安全返回 nil。
--
-- 预期：foo(1) 后 g={x=1}，bar() 读 g.a=nil → 0；foo(0) 后 g={y=2}，bar() 读 g.a=nil → 0。
local g = { a = 100 }

function test_global_crossfunc(cond)
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
