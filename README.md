#fakescript

[![Author](https://img.shields.io/badge/author-errhrs-blue.svg?style=flat)](https://github.com/esrrhs/fakescript) [![Platform](https://img.shields.io/badge/platform-Linux,%20Windows-green.svg?style=flat)](https://github.com/esrrhs/fakescript)[![Script](https://img.shields.io/badge/embed-script-pink.svg?style=flat)](https://github.com/esrrhs/fakescript) [![License](https://img.shields.io/github/license/mashape/apistatus.svg?maxAge=2592000?style=flat)](LICENSE)

Lightweight embedded scripting language

## Brief introduction [中文] (./README_CN.md)
**fakescript** is a lightweight embedded scripting language , using c ++ language, grammar lessons from lua, golang, erlang, based on flex, bison generative grammar tree , compiled into byte code interpreted. Compared with lua, comparable speed , easier to use , more readable source code
<a href="https://github.com/esrrhs/fakescript-java">fakescript for Java</a>

## Script feature
* The syntax is similar to lua
* All function
* Support array, map, unlimited nested
* Support generating routine like fake testfunc (param1), to achieve the effect of multithreading
* Support binding C functions and class member functions
* With interpreter, JIT part support
* Support multi return value
* Support profile, can get the script running time of each function
* Support hot update
* Support Int64
* Support const definitions
* Support Package
* Support struct
* Support packaged bin file or executable file
* Own command line style debugger like gdb, and vs style debugging ide
* Can start a command-line debugging in C through the interface call



## In Production

<a href="http://dzz.youxi.com/"><img src="https://ss1.baidu.com/70cFfyinKgQFm2e88IuM_a/forum/pic/item/d8f9d72a6059252d8aa6c584329b033b5ab5b9f9.jpg" width="84" align="middle"/></a>&nbsp;&nbsp;
<a href="http://ttxd.qq.com/act/a20160419brandP/"><img src="https://ss1.baidu.com/70cFfyinKgQFm2e88IuM_a/forum/pic/item/9f510fb30f2442a71a9ae83ad443ad4bd013029c.jpg" width="100" align="middle"/></a>&nbsp;&nbsp;

## Standard Library
* File
* Network
* String
* Xml
* Template
* Json

## Sample

```
#!lua


-- Current package name
package mypackage.test

-- include file
include "common.fk"

-- struct define
struct teststruct
	sample_a
	sample_b
	sample_c
end

-- const define
const hellostring = "hello"
const helloint = 1234
const hellomap = {1 : "a" 2 : "b" 3 : [1 2 3]}

-- func1 comment
func myfunc1(arg1, arg2)
	
	-- C function calls and class member functions
	arg3 := cfunc1(helloint) + arg2:memfunc1(arg1)
	
	-- Branch
	if arg1 < arg2 then	
		-- create routine
		fake myfunc2(arg1, arg2)
	elseif arg1 == arg2 then	
		print("elseif")
	else
		print("else")
	end
	
	-- for loop
	for var i = 0, i < arg2, i++ then
		print("i = ", i)
	end
	
	-- array use
	var a = array()
	a[1] = 3
	
	-- map use
	var b = map()
	b[a] = 1
	b[1] = a
	
	-- Int64
	var uid = 1241515236123614u
	log("uid = ", uid)

	-- sub func call
	var ret1, var ret2 = myfunc2()

	-- other package call
	ret1 = otherpackage.test.myfunc1(arg1, arg2)
	
	-- struct use
	var tt = teststruct()
	tt->sample_a = 1
	tt->sample_b = teststruct()
	tt->sample_b->sample_a = 10

	-- switch branch
	switch arg1
		case 1 then
			print("1")
		case "a" then
			print("a")
		default
			print("default")
	end

	-- multi return value
	return arg1, arg3
	
end
```

# C++ Sample #

```
#!c++
// create instance
fake * fk = newfake();
// regist global func
fkreg(fk, "cfunc1", cfunc1);
// regist class mem func, diffent class type can regist the same name
fkreg(fk, "memfunc1", &class1::memfunc1);
// parse script file
fkparse(fk, argv[1]);
// run script func myfunc1, put in two param 1 and 2
ret = fkrun<int>(fk, "myfunc1", 1, 2);
// delete instance
delfake(fk);
```

## Debugging environment
* IDE

![QQ图片20150819174501.png](https://bitbucket.org/repo/pAEqqM/images/188608728-QQ%E5%9B%BE%E7%89%8720150819174501.png)

* Command-line

![QQ图片20150819175022.png](https://bitbucket.org/repo/pAEqqM/images/1059284915-QQ%E5%9B%BE%E7%89%8720150819175022.png)

## Welcome to donate 
![donation.png](https://bitbucket.org/repo/pAEqqM/images/2041212420-donation.png)

Donated money will be used to improve performance and subsequent continuous optimization fakescript
