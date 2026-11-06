#include "benchmark/benchmark.h"
#include "fakelua.h"

#include <string>

using namespace fakelua;

// NativeObject 热路径基准：字段读写、对象创建、父子绑定与深树叶子写入。
// 这些用例只使用长期存在的公开 API，因此同一份基准可分别跑在加版本号前后的构建上。

static void BM_NativeObjectSetInt(benchmark::State &state) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *obj = mgr.Create(gid, "bench", 1);
    for (auto _: state) {
        obj->SetInt("hp", 42);
        benchmark::DoNotOptimize(obj);
    }
    mgr.Clear();
    FakeluaDeleteState(s);
}

static void BM_NativeObjectGetInt(benchmark::State &state) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *obj = mgr.Create(gid, "bench", 1);
    obj->SetInt("hp", 42);
    for (auto _: state) {
        benchmark::DoNotOptimize(obj->GetInt("hp"));
    }
    mgr.Clear();
    FakeluaDeleteState(s);
}

static void BM_NativeObjectSetString(benchmark::State &state) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *obj = mgr.Create(gid, "bench", 1);
    const std::string value = "benchmark-value";
    for (auto _: state) {
        obj->SetString("name", value);
        benchmark::DoNotOptimize(obj);
    }
    mgr.Clear();
    FakeluaDeleteState(s);
}

static void BM_NativeObjectCreateDestroy(benchmark::State &state) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    int64_t id = 0;
    for (auto _: state) {
        auto *obj = mgr.Create(gid, "bench", ++id);
        benchmark::DoNotOptimize(obj);
        mgr.DestroySingle("bench", id);
    }
    mgr.Clear();
    FakeluaDeleteState(s);
}

static void BM_NativeObjectSetObject(benchmark::State &state) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    auto *parent = mgr.Create(gid, "bench_parent", 1);
    auto *child = mgr.Create(gid, "bench_child", 2);
    for (auto _: state) {
        parent->SetObject("child", child);
    }
    mgr.Clear();
    FakeluaDeleteState(s);
}

// 8 层父子链上写叶子：用于观察深层对象上的写入开销（简化后应与浅层写入一致）。
static void BM_NativeObjectDeepLeafWrite(benchmark::State &state) {
    auto *s = FakeluaNewState();
    auto &mgr = GetNativeObjectManager(s);
    int64_t gid = mgr.CreateGroup();
    NativeObject *cur = mgr.Create(gid, "bench_node", 1);
    for (int i = 0; i < 8; ++i) {
        auto *child = mgr.Create(gid, "bench_node", i + 2);
        cur->SetObject("child", child);
        cur = child;
    }
    for (auto _: state) {
        cur->SetInt("leaf", 1);
        benchmark::DoNotOptimize(cur);
    }
    mgr.Clear();
    FakeluaDeleteState(s);
}

BENCHMARK(BM_NativeObjectSetInt);
BENCHMARK(BM_NativeObjectGetInt);
BENCHMARK(BM_NativeObjectSetString);
BENCHMARK(BM_NativeObjectCreateDestroy);
BENCHMARK(BM_NativeObjectSetObject);
BENCHMARK(BM_NativeObjectDeepLeafWrite);
