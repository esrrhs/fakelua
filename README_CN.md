#fakescript 
轻量级嵌入式脚本语言

## 简介
**fakescript**是一款轻量级的嵌入式脚本语言，使用c++语言编写，语法吸取自lua、golang、erlang，基于flex、bison生成语法树，编译成字节码解释执行。与lua相比，运行速度不相上下，更易使用，源代码可读性更强
<a href="https://github.com/esrrhs/fakescript-java">Java版本fakescript</a>

## 脚本特性
* 语法类似lua
* 全部为函数
* 支持array，map，可以无限嵌套
* 支持fake testfunc(param1)产生routine，实现假多线程效果
* 支持C函数和类成员函数的绑定
* 自带解释器，JIT部分支持
* 支持多返回值
* 自带profile，可获取脚本各个函数运行时间
* 支持热更新
* 支持Int64
* 支持const定义
* 支持包
* 支持struct
* 支持打包bin文件或可执行文件
* 自带gdb风格的命令行调试器，以及vs风格的可视化编辑调试ide
* 可在C里直接通过接口调用，开始命令行调试

## 标准库
* 文件
* 网络
* 字符串
* xml
* 模板类
* json

## 示例

```
#!lua


-- 当前包名
package mypackage.test

-- 引入的文件
include "common.fk"

-- 结构体定义
struct teststruct
	sample_a
	sample_b
	sample_c
end

-- 常量值
const hellostring = "hello"
const helloint = 1234
const hellomap = {1 : "a" 2 : "b" 3 : [1 2 3]}

-- func1 comment
func myfunc1(arg1, arg2)
	
	-- C函数和类成员函数的调用
	arg3 := cfunc1(helloint) + arg2:memfunc1(arg1)
	
	-- 分支
	if arg1 < arg2 then	
		-- 创建一个协程
		fake myfunc2(arg1, arg2)
	elseif arg1 == arg2 then	
		print("elseif")
	else
		print("else")
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

	-- 其他包的函数调用
	ret1 = otherpackage.test.myfunc1(arg1, arg2)
	
	-- 结构体
	var tt = teststruct()
	tt->sample_a = 1
	tt->sample_b = teststruct()
	tt->sample_b->sample_a = 10

	-- 分支
	switch arg1
		case 1 then
			print("1")
		case "a" then
			print("a")
		default
			print("default")
	end

	-- 多返回值
	return arg1, arg3
	
end
```

## C++示例

```
#!c++
// 创建一个实例
fake * fk = newfake();
// 注册全局函数
fkreg(fk, "cfunc1", cfunc1);
// 注册类成员函数，不同的类注册一样的函数名字不冲突
fkreg(fk, "memfunc1", &class1::memfunc1);
// 解析fake脚本文件
fkparse(fk, argv[1]);
// 执行myfunc1函数，传入两个参数分别为1和2
ret = fkrun<int>(fk, "myfunc1", 1, 2);
// 删除实例
delfake(fk);
```

## 调试环境
* IDE

![QQ图片20150819174501.png](https://bitbucket.org/repo/pAEqqM/images/188608728-QQ%E5%9B%BE%E7%89%8720150819174501.png)

* 命令行

![QQ图片20150819175022.png](https://bitbucket.org/repo/pAEqqM/images/1059284915-QQ%E5%9B%BE%E7%89%8720150819175022.png)

## 欢迎捐赠
![donation.png](https://bitbucket.org/repo/pAEqqM/images/2041212420-donation.png)

欢迎使用支付宝手扫描上面的二维码，对该项目进行捐赠。捐赠款项将用于改进fakescript性能以及后续持续优化。

