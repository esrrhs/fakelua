#pragma once

#include <string>
#include <string_view>

namespace fakelua {

// GCC JIT 编译产物的磁盘缓存。
//
// 背景：在 macOS 上，首次 dlopen 一个新路径的 dylib 需要约 4.5~5.4 秒走代码签名验证
// （amfid / syspolicyd IPC），而同路径的后续加载只要约 0.2 毫秒。当同一份 C 代码被
// 反复编译时（例如测试里 JIT 类型 × debug 模式的组合），这笔固定开销会成倍累积，
// 使测试套件慢到不可用。
//
// 做法：以「生成的 C 代码 + 影响产物的编译参数」的摘要作为缓存键，命中时直接复用
// 同一路径的 .so，从而命中 dyld 的路径缓存。实测 dlclose 之后重新 dlopen 同一路径
// 仍是 0.2 毫秒，说明 dyld 的记账按路径且不随 dlclose 失效，因此缓存文件可以走
// 正常的 GCCHandle 生命周期，无需常驻内存。

// 计算缓存摘要。相同输入必定返回相同字符串。
std::string ComputeGccCacheKey(std::string_view c_code, std::string_view compile_flags);

// 返回缓存目录下该摘要对应的产物路径（不保证文件已存在）。
// suffix 为 ".dylib" / ".so" / ".dll"。
std::string GccCacheArtifactPath(std::string_view key, std::string_view suffix);

// 返回缓存根目录，并确保目录已创建。失败时返回空字符串表示不可用。
std::string GccCacheDir();

}// namespace fakelua
