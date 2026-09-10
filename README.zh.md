# fake

[<img src="https://img.shields.io/github/license/esrrhs/fake">](https://github.com/esrrhs/fake)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fake">](https://github.com/esrrhs/fake)
[<img src="https://img.shields.io/github/v/release/esrrhs/fake">](https://github.com/esrrhs/fake)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/450723e1bc374ccd8aac7154227c69d7)](https://www.codacy.com/manual/esrrhs/fake?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=esrrhs/fake&amp;utm_campaign=Badge_Grade)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fake/ccpp.yml?branch=master">](https://github.com/esrrhs/fake/actions)

中文 | [English](README.md)

**fake** 是一款轻量级嵌入式脚本语言，用 C++ 编写。语法吸取自 Lua、Go、Erlang；基于 flex、bison 生成语法树，编译成字节码在 VM 上解释执行，并提供实验性 JIT。

其他实现：[Java 版](https://github.com/esrrhs/fakejava) · [Go 版](https://github.com/esrrhs/fakego)

## 内存模型

fake **没有垃圾回收**。字符串、array、map、绑定指针会一直活到 `fake` 实例结束。调用 `delfake(fk)` 时一口气释放全部内存。

## 特性

- **运行环境**：Linux amd64、macOS amd64；字节码 VM + 实验性 JIT
- **并发**：`fake fn(args)` 在单线程上创建 routine（JIT 下不可用）
- **互操作**：绑定 C 函数和 C++ 成员函数；支持热更新
- **语言**：包、`include`、`struct`、`const`、可嵌套的 `array` / `map`、多返回值、Int64
- **工具**：gdb 风格命令行调试器、VS 风格可视化 IDE、函数 profile
- **打包**：可打成 bin 或独立可执行文件

## 嵌入

把 `include/fake-inc.h` 和 `bin/libfake.so` 拷进工程即可。

```cpp
fake * fk = newfake();
fkreg(fk, "cfunc1", cfunc1);
fkreg(fk, "memfunc1", &class1::memfunc1);  // 不同类注册同名函数不冲突
fkparse(fk, argv[1]);
int ret = fkrun<int>(fk, "myfunc1", 1, 2);
delfake(fk);
```

## 语言

```
-- 当前包名
package mypackage.test

-- 引入的文件
include "common.fk"

struct teststruct
	sample_a
	sample_b
	sample_c
end

const hellostring = "hello"
const helloint = 1234
const hellomap = {1 : "a" 2 : "b" 3 : [1 2 3]}

func myfunc1(arg1, arg2)

	-- C 函数和类成员函数调用
	arg3 := cfunc1(helloint) + arg2:memfunc1(arg1)

	if arg1 < arg2 then
		fake myfunc2(arg1, arg2)   -- 创建一个协程
	elseif arg1 == arg2 then
		print("elseif")
	else
		print("else")
	end

	for var i = 0, i < arg2, i++ then
		print("i = ", i)
	end

	var a = array()
	a[1] = 3

	var b = map()
	b[a] = 1
	b[1] = a

	var uid = 1241515236123614u
	log("uid = ", uid)

	var ret1, var ret2 = myfunc2()
	ret1 = otherpackage.test.myfunc1(arg1, arg2)

	var tt = teststruct()
	tt->sample_a = 1
	tt->sample_b = teststruct()
	tt->sample_b->sample_a = 10

	switch arg1
		case 1 then
			print("1")
		case "a" then
			print("a")
		default
			print("default")
	end

	return arg1, arg3
end
```

## 性能

```bash
cd benchmark/ && ./benchmark.sh
```

在 MacBook Pro 2.3 GHz Intel Core i5 上的数据：

|        | Lua   | Python | Fake | Fake JIT |
|--------|-------|:------:|-----:|---------:|
| Loop   | 0.8s  | 2.3s   | 1.3s | 0.2s     |
| Prime  | 13.5s | 20.9s  | 12.8s | 5.9s    |
| String | 0.8s  | 0.4s   | 1.2s | 3.2s     |

## 编译

1. 安装 cmake、gcc、g++
2. （可选）安装 flex、bison，运行 `./gen.sh` 重新生成解析器
3. `./build.sh` 或 `./build.sh release`

## 测试

示例脚本在 `test/sample`。

```bash
cd test && ./test.sh          # VM
cd test && ./test.sh -j       # JIT
./bin/fakebin your.fk         # 运行脚本
```

## 调试

- IDE：`bin/fakeide.app`

![ide](img/ide.png)

- 命令行：`bin/fakebin`

![debug](img/debug.png)

## Stargazers over time

[![Stargazers over time](https://starchart.cc/esrrhs/fake.svg)](https://starchart.cc/esrrhs/fake)
