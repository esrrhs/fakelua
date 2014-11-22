# fakescript #
fakescript是一款轻量级的嵌入式脚本语言，与Lua相比，它的运行速度更快，更容易使用，同时源代码可读性更强。

脚本特性：
@.代码风格类似lua
@.全部为函数
@.支持array，map，可以无限嵌套
@.支持fake testfunc(param1)产生routine，实现假多线程效果
@.支持C函数和类成员函数的绑定
@.自带解释器，JIT部分支持
@.支持多返回值
@.自带profile，可获取脚本各个函数运行时间
@.支持热更新
@.支持Int64

示例：
-- func1 comment
func myfunc1(arg1, arg2)
	
	-- C函数和类成员函数的调用
	var arg3 = cfunc1(arg1) + arg2:memfunc1(arg1)
	
	-- 分支
	if arg1 < arg2 then	
		-- 创建一个协程
		fake myfunc2(arg1, arg2)
	end
	
	-- for循环
	for var i = 0, i < arg2, i++ then
		print("i = ", i)
	end
	
	-- 数组
	var a = array()
	a[1] = 3
	
	-- 集合
	var b = map()
	b[a] = 1
	b[1] = a
	
	-- Int64
	var uid = 1241515236123614u
	log("uid = ", uid)

	-- 子函数调用
	var ret1, var ret2 = myfunc2()

	-- 多返回值
	return arg1, arg3
	
end