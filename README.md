# FakeLua(Work in process)
FakeLua是一个可以优化Lua执行的工具。通过运行时将Lua编译成机器码，牺牲部分Lua特性，以提高Lua执行速度。

# 特性
* C++20编译
* Flex、Bison做词法语法分析
* 并发API，不支持_G等全局状态
* 统一内存管理，无GC
* 使用GCC JIT生成机器码执行

