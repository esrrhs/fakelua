#pragma once
// Stub header for libgccjit++ - real implementation is commented out in gcc_jit.cpp

struct gcc_jit_block;
struct gcc_jit_result;

namespace gccjit {

struct context {
    void release() {}
};

struct location {};
struct type {};
struct field {};
struct lvalue {};
struct rvalue {};
struct block {};

struct function {
    block new_block(const char*) { return {}; }
};

}  // namespace gccjit
