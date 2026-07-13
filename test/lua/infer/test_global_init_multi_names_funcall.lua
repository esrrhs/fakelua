function func()
    return 10, 20
end
-- 文件级 local，names(2) != exps(1)：走 PreprocessGlobalInitializers 的 else 分支，
-- 整条 a, b = func() 推进 __fakelua_init()。必须保留 func() 调用，不能丢失成 nil。
local a, b = func()
function test()
    return a + b
end
