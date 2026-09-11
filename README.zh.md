# fake

[<img src="https://img.shields.io/github/license/esrrhs/fake">](https://github.com/esrrhs/fake)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fake">](https://github.com/esrrhs/fake)
[<img src="https://img.shields.io/github/v/release/esrrhs/fake">](https://github.com/esrrhs/fake)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/450723e1bc374ccd8aac7154227c69d7)](https://www.codacy.com/manual/esrrhs/fake?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=esrrhs/fake&amp;utm_campaign=Badge_Grade)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fake/ccpp.yml?branch=master">](https://github.com/esrrhs/fake/actions)

中文 | [English](README.md)

轻量级嵌入式脚本语言，用 C++ 编写。语法吸取自 Lua、Go、Erlang。脚本经 flex/bison 解析，编译成字节码在 VM 上执行（可选实验性 JIT）。

## 特性

- 字节码 VM 与实验性 JIT（Linux / macOS amd64）
- 绑定 C 函数和 C++ 成员函数；支持热更新
- 包、`include`、`struct`、`const`、可嵌套的 `array` / `map`、多返回值、Int64
- `fake fn(args)` 在单线程上创建 routine（JIT 下不可用）
- gdb 风格命令行调试器、可视化 IDE、函数 profile
- 可打成 bin 或独立可执行文件
- 没有垃圾回收 — 对象活到 `delfake()`，销毁时一口气释放全部内存

## 依赖

- cmake、gcc、g++
- 只有需要重新生成解析器时才要 flex、bison（`./gen.sh`）

## 编译

```bash
./build.sh           # debug
./build.sh release   # 优化
```

产物是 `bin/libfake.so` 和 `bin/fakebin`。

## 使用

运行脚本：

```bash
./bin/fakebin your.fk
```

示例在 `test/sample`。测试：

```bash
cd test && ./test.sh      # VM
cd test && ./test.sh -j   # JIT
```

嵌入 C++ — 把 `include/fake-inc.h` 和 `bin/libfake.so` 拷进工程：

```cpp
fake *fk = newfake();
fkreg(fk, "cfunc1", cfunc1);
fkreg(fk, "memfunc1", &class1::memfunc1);  // 不同类注册同名函数不冲突
fkparse(fk, argv[1]);
int ret = fkrun<int>(fk, "myfunc1", 1, 2);
delfake(fk);  // 释放全部内存
```

## 语言

```
package mypackage.test
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
	arg3 := cfunc1(helloint) + arg2:memfunc1(arg1)

	if arg1 < arg2 then
		fake myfunc2(arg1, arg2)
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

## 调试

IDE（`bin/fakeide.app`）：

![ide](img/ide.png)

命令行（`bin/fakebin`）：

![debug](img/debug.png)

## 基准

```bash
cd benchmark && ./benchmark.sh
```

MacBook Pro 2.3 GHz Intel Core i5：

|        | Lua   | Python | Fake  | Fake JIT |
|--------|-------|--------|------:|---------:|
| Loop   | 0.8s  | 2.3s   | 1.3s  | 0.2s     |
| Prime  | 13.5s | 20.9s  | 12.8s | 5.9s     |
| String | 0.8s  | 0.4s   | 1.2s  | 3.2s     |

## 相关项目

- [fakejava](https://github.com/esrrhs/fakejava)
- [fakego](https://github.com/esrrhs/fakego)

## 许可证

[MIT](LICENSE)

## Stargazers over time

[![Stargazers over time](https://starchart.cc/esrrhs/fake.svg)](https://starchart.cc/esrrhs/fake)
