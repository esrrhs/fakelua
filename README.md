# fake

[<img src="https://img.shields.io/github/license/esrrhs/fake">](https://github.com/esrrhs/fake)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fake">](https://github.com/esrrhs/fake)
[<img src="https://img.shields.io/github/v/release/esrrhs/fake">](https://github.com/esrrhs/fake)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/450723e1bc374ccd8aac7154227c69d7)](https://www.codacy.com/manual/esrrhs/fake?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=esrrhs/fake&amp;utm_campaign=Badge_Grade)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fake/ccpp.yml?branch=master">](https://github.com/esrrhs/fake/actions)

[中文](README.zh.md) | English

Lightweight embeddable scripting language in C++. Syntax is borrowed from Lua, Go, and Erlang. Scripts are parsed with flex/bison, compiled to bytecode, and run on a VM.

## Features

- Bytecode VM (Linux / macOS amd64)
- Bind C functions and C++ member functions; re-register the same name to hot-reload
- Packages, `include`, `struct`, `const`, nested `array` / `map`, multiple return values, Int64
- Single-thread routines via `fake fn(args)`
- gdb-style CLI debugger, function profiler
- Pack scripts into a bin or a standalone executable
- No garbage collector — runtime objects live until `fkreset()` or `delfake()`
- `fkrun` runs until the function returns; infinite loops hang. No paused VM for the host to tick.

## Requirements

- CMake 3.16+, a C++14 compiler (gcc/g++ or clang)
- flex and bison only if you need to regenerate the parser (`./gen.sh`)

## Build

```bash
./build.sh           # debug
./build.sh release   # optimized
```

This produces `bin/libfake.so`, `bin/fakebin`, `bin/fake_tests`, and `bin/fake_bench`.
The first configure downloads GoogleTest and Google Benchmark.

## Usage

Run a script:

```bash
./bin/fakebin your.fk
```

Examples are in `test/sample` (descriptive names, not numbers). After a build:

```bash
ctest --output-on-failure
./bin/fake_tests --gtest_filter=Language.*
```

CI (Linux/macOS × debug/release) runs GoogleTest and Google Benchmark on every push.

Embed in C++ — copy `include/fake-inc.h` and `bin/libfake.so` into your project:

```cpp
fake *fk = newfake();
fkreg(fk, "cfunc1", cfunc1);
fkreg(fk, "memfunc1", &class1::memfunc1);  // same name on different classes does not clash
fkparse(fk, argv[1]);
int ret = fkrun<int>(fk, "myfunc1", 1, 2);  // runs to completion; does not leave a paused VM
fkparse(fk, argv[1]);  // parse or fkreg again — same name is replaced
fkreset(fk);  // drop runtime arrays/maps/pointers/strings; keep bytecode and C bindings
delfake(fk);  // destroy the instance and free everything
```

### Lifetime

| Call | What it frees | What it keeps |
|------|----------------|---------------|
| `fkparse` / `fkreg` again | Previous bytecode or C binding of that name | Everything else |
| `fkreset(fk)` | Runtime arrays, maps, pointer wrappers, interned runtime strings, stacks | Bytecode, constants, registered C functions |
| `fkclear(fk)` | All compiled bytecode | C bindings |
| `delfake(fk)` | The whole instance | — |

Call `fkreset()` only when no script is running. `fkrun` always finishes (or hangs) before it returns to the host. Do not replace a function that is currently on the call stack.

## Language

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

## Debugging

CLI (`bin/fakebin`):

![debug](img/debug.png)

## Benchmarks

Google Benchmark microbenches (`loop`, `call`, `array`, `string`, `parse`):

```bash
./bin/fake_bench
# or: cd benchmark && ./benchmark.sh
```

`benchmark/*.lua` and `*.py` remain if you want a manual cross-language comparison.

MacBook Pro 2.3 GHz Intel Core i5 (older `benchmark.sh` numbers):

|        | Lua   | Python | Fake  |
|--------|-------|--------|------:|
| Loop   | 0.8s  | 2.3s   | 1.3s  |
| Prime  | 13.5s | 20.9s  | 12.8s |
| String | 0.8s  | 0.4s   | 1.2s  |

## Related projects

- [fakejava](https://github.com/esrrhs/fakejava)
- [fakego](https://github.com/esrrhs/fakego)

## License

[MIT](LICENSE)

## Stargazers over time

[![Stargazers over time](https://starchart.cc/esrrhs/fake.svg)](https://starchart.cc/esrrhs/fake)
