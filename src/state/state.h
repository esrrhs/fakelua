#pragma once

#include "compile/compiler.h"
#include "fakelua.h"
#include "jit/vm.h"
#include "state/const_string.h"
#include "state/heap.h"
#include "util/logging.h"

#include <memory>
#include <typeindex>
#include <unordered_map>

struct TCCState;

namespace fakelua {

namespace native {
class IoContext;
}

// JIT 错误边界，完整定义在 jit/jit_error_boundary.h
struct JitErrorBoundary;

// 单个运行实例
// 线程模型：State 是单线程实体，由调用方保证同一时刻只有一个线程访问它。
// 本类所有成员（包括 reentrant_count_ 的 Add/Sub、heap_、vm_ 等）都未加锁：
//  - reentrant_count_ 只在脚本执行入口/出口处自增自减，用来检测重入而非跨线程计数；
//  - heap_ / vm_ / const_string_ 也依赖调用方的线程亲和性。
// 如果将来需要跨线程共享同一 State，需要在此处引入同步或改为每线程一份 State。
class State {
public:
    explicit State(const StateConfig &config = {});
    ~State();

    void Reset() {
        DEBUG_ASSERT(reentrant_count_ == 0);
        heap_.Reset();
    }

    [[nodiscard]] const StateConfig &GetStateConfig() const {
        return config_;
    }

    Compiler &GetCompiler() {
        return compiler_;
    }

    Heap &GetHeap() {
        return heap_;
    }

    ConstString &GetConstString() {
        return const_string_;
    }

    Vm &GetVM() {
        return vm_;
    }

    [[nodiscard]] int GetReentrantCount() const {
        return reentrant_count_;
    }

    // 单线程调用（见类注释），无需原子操作。
    void AddReentrantCount() {
        ++reentrant_count_;
    }

    // 单线程调用（见类注释），无需原子操作。
    void SubReentrantCount() {
        --reentrant_count_;
    }

    void SetVarInterfaceNewFunc(const std::function<VarInterface *()> &func) {
        DEBUG_ASSERT(func != nullptr);
        var_interface_new_func_ = func;
    }

    std::function<VarInterface *()> &GetVarInterfaceNewFunc() {
        return var_interface_new_func_;
    }

    // native 模块（net、mysql）共用的 asio 事件循环。按需创建，因为多数 State 根本
    // 用不到 native IO，而一个 io_context 要占一个 epoll fd / IOCP 句柄。
    native::IoContext &GetIoContext();

    // 本 State 的原生对象管理器。它是 State 的一部分，不存在游离的实例（构造函数私有，
    // 只有 State 能建）：对象、分组、全局对象、id 发号都不跨 State 共享。
    NativeObjectManager &GetNativeObjectManager() {
        return native_objects_;
    }

    // JIT 错误边界链的栈顶，为空表示当前不在 JIT 代码执行过程中。
    // 边界本身在 C++ 栈上，这里只存链顶指针，由 JitErrorBoundaryScope 维护。
    [[nodiscard]] JitErrorBoundary *GetJitErrorBoundary() const {
        return jit_error_boundary_;
    }

    void SetJitErrorBoundary(JitErrorBoundary *boundary) {
        jit_error_boundary_ = boundary;
    }

    // 解释器执行 __fakelua_init 期间为 true：此时分配走常量堆，帧 Reset 后仍然有效。
    void SetInterpConstAlloc(bool v) {
        interp_const_alloc_ = v;
    }

    [[nodiscard]] bool InterpConstAlloc() const {
        return interp_const_alloc_;
    }

    // 本 State 的日志输出目标。为 nullptr 表示没指定日志文件，只打控制台。
    LogSink *GetLogSink() const {
        return log_sink_.get();
    }

    [[nodiscard]] LogLevel GetLogLevel() const {
        return log_level_;
    }

    void SetLogLevel(LogLevel level) {
        log_level_ = level;
    }

    void SetLogFile(const std::string &path, size_t max_size = 10 * 1024 * 1024, size_t max_files = 5);

    // 某个 native 模块在本 State 上的私有状态，首次访问时创建，随 State 销毁。
    // 以前各模块把它存在自己文件里的 static unordered_map<State *, X> 中，那是全进程
    // 一份：两个线程各跑自己的 State（见上面的线程模型），同时创建对象就会并发改同一个
    // map，一边 rehash 一边 find 是未定义行为。挪到 State 上之后没有共享容器，也就不需
    // 要加锁。
    // 类型本身就是键，所以每个模块必须用自己专属的类型，不能直接拿 std::vector 这类
    // 通用类型当状态，否则两个模块会撞进同一个槽。
    template<typename T>
    T &GetModuleState() {
        const auto key = std::type_index(typeid(T));
        auto it = module_states_.find(key);
        if (it == module_states_.end()) {
            ModuleStateHolder holder(new T(), [](void *p) { delete static_cast<T *>(p); });
            it = module_states_.emplace(key, std::move(holder)).first;
        }
        return *static_cast<T *>(it->second.get());
    }

    // 只查不建，没有则返回 nullptr。清理路径上用这个：那时再创建一个空状态没有意义，而
    // FakeluaDeleteState 会无条件调用每个模块的清理，用上面那个会给每个 State 白建一堆
    // 从没用过的模块状态。
    template<typename T>
    T *TryGetModuleState() {
        const auto it = module_states_.find(std::type_index(typeid(T)));
        return it == module_states_.end() ? nullptr : static_cast<T *>(it->second.get());
    }

private:
    // 类型擦除的持有者：State 不认识各模块的状态类型，靠这个函数指针在销毁时按真实
    // 类型析构。
    using ModuleStateHolder = std::unique_ptr<void, void (*)(void *)>;

    // 声明在最前：销毁是声明的逆序，所以它最后才析构 —— 其他成员析构的时候还可能打日志
    // （比如原生对象的 finalizer）。用函数指针删除器是为了不必在这里看到 LogSink 的定义。
    std::unique_ptr<LogSink, void (*)(LogSink *)> log_sink_{nullptr, nullptr};
    LogLevel log_level_ = LogLevel::Info;

    std::function<VarInterface *()> var_interface_new_func_;
    JitErrorBoundary *jit_error_boundary_ = nullptr;
    bool interp_const_alloc_ = false;
    int reentrant_count_ = 0;
    StateConfig config_;
    Compiler compiler_;
    Heap heap_;
    ConstString const_string_;
    Vm vm_;

    // 下面三个的声明顺序是有讲究的：销毁是声明的逆序，而它们之间有依赖，写反会在
    // ~State 里踩到已经析构的成员。
    // 依赖关系（销毁的先后）：
    //  1. native_objects_ 最先销毁。它的 Clear() 会回调进各模块（如 io）去清掉指向这些
    //     对象的缓存，所以那时 module_states_ 必须还在。
    //  2. module_states_ 次之。里面只是一堆 NativeObject 裸指针，析构时不会去碰对象本身。
    //  3. io_context_ 最后。原生对象的 finalizer 要停 socket、关连接，那些都跑在它上面。
    // 常规路径上其实轮不到这里：FakeluaDeleteState 会在 delete state 之前显式做完各模块
    // 的清理。这个顺序是给直接 new/delete State 的用法兜底的。
    std::unique_ptr<native::IoContext> io_context_;
    std::unordered_map<std::type_index, ModuleStateHolder> module_states_;
    NativeObjectManager native_objects_{this};
};

}// namespace fakelua
