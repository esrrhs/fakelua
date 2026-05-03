#include "compile/compiler.h"
#include "fakelua.h"
#include "jit/vm.h"
#include "jit/vm_function.h"
#include "state/const_string.h"
#include "state/state.h"
#include "var/var.h"
#include "var/var_string.h"
#include "gtest/gtest.h"

using namespace fakelua;

// ---------------------------------------------------------------------------
// ConstString tests
// ---------------------------------------------------------------------------

TEST(state, const_string_alloc_and_interning) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();
    auto &cs = s->GetConstString();

    const int64_t id1 = cs.Alloc("hello");
    const int64_t id2 = cs.Alloc("world");
    const int64_t id3 = cs.Alloc("hello"); // should return same id as id1

    ASSERT_NE(id1, id2);
    ASSERT_EQ(id1, id3); // interning: same string → same ID
}

TEST(state, const_string_get_string_roundtrip) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();
    auto &cs = s->GetConstString();

    const int64_t id = cs.Alloc("fakelua");
    const std::string_view sv = ConstString::GetString(id);
    ASSERT_EQ(sv, "fakelua");
}

TEST(state, const_string_get_var_string) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();
    auto &cs = s->GetConstString();

    const int64_t id = cs.Alloc("teststr");
    const VarString *vs = ConstString::GetVarString(id);
    ASSERT_NE(vs, nullptr);
    ASSERT_EQ(vs->Str(), "teststr");
}

TEST(state, const_string_size) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();
    auto &cs = s->GetConstString();

    const size_t before = cs.Size();
    cs.Alloc("unique_key_abc");
    ASSERT_EQ(cs.Size(), before + 1u);

    // Reallocating same string must not increase size
    cs.Alloc("unique_key_abc");
    ASSERT_EQ(cs.Size(), before + 1u);
}

// ---------------------------------------------------------------------------
// GetFuncAddr tests
// ---------------------------------------------------------------------------

static CVar dummy_fn() {
    Var r;
    r.SetInt(42);
    return r;
}

TEST(state, get_func_addr_success) {
    State s;
    s.GetVM().RegisterFunction(VmFunction("myfunc", 0, JIT_TCC, reinterpret_cast<void *>(&dummy_fn), {}));

    int arg_count = -1;
    void *addr = inter::GetFuncAddr(&s, JIT_TCC, "myfunc", arg_count);
    ASSERT_NE(addr, nullptr);
    ASSERT_EQ(arg_count, 0);
    ASSERT_EQ(addr, reinterpret_cast<void *>(&dummy_fn));
}

TEST(state, get_func_addr_missing_returns_null) {
    State s;
    int arg_count = -1;
    void *addr = inter::GetFuncAddr(&s, JIT_TCC, "nonexistent", arg_count);
    ASSERT_EQ(addr, nullptr);
    // arg_count must remain unchanged when function is not found
    ASSERT_EQ(arg_count, -1);
}

// ---------------------------------------------------------------------------
// Reentrant count tests
// ---------------------------------------------------------------------------

TEST(state, reentrant_count_increment_decrement) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    ASSERT_EQ(inter::GetReentrantCount(s), 0);

    inter::AddReentrantCount(s);
    ASSERT_EQ(inter::GetReentrantCount(s), 1);

    inter::AddReentrantCount(s);
    ASSERT_EQ(inter::GetReentrantCount(s), 2);

    inter::SubReentrantCount(s);
    ASSERT_EQ(inter::GetReentrantCount(s), 1);

    inter::SubReentrantCount(s);
    ASSERT_EQ(inter::GetReentrantCount(s), 0);
}

// ---------------------------------------------------------------------------
// inter::Reset tests
// ---------------------------------------------------------------------------

TEST(state, reset_clears_heap) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    // Allocate some temp memory so that the heap is non-empty.
    auto &temp_alloc = s->GetHeap().GetTempAllocator();
    ASSERT_NE(temp_alloc.Alloc(256), nullptr);
    ASSERT_GT(temp_alloc.Size(), 0u);

    inter::Reset(s);
    ASSERT_EQ(temp_alloc.Size(), 0u);
}

// ---------------------------------------------------------------------------
// SetVarInterfaceNewFunc / GetVarInterfaceNewFunc tests
// ---------------------------------------------------------------------------

struct TestVarImpl final : public VarInterface {
    [[nodiscard]] Type ViGetType() const override { return Type::NIL; }
    void ViSetNil() override {}
    void ViSetBool(bool) override {}
    void ViSetInt(int64_t) override {}
    void ViSetFloat(double) override {}
    void ViSetString(const std::string_view &) override {}
    void ViSetTable(const std::vector<std::pair<VarInterface *, VarInterface *>> &) override {}
    [[nodiscard]] bool ViGetBool() const override { return false; }
    [[nodiscard]] int64_t ViGetInt() const override { return 0; }
    [[nodiscard]] double ViGetFloat() const override { return 0; }
    [[nodiscard]] std::string_view ViGetString() const override { return {}; }
    [[nodiscard]] size_t ViGetTableSize() const override { return 0; }
    [[nodiscard]] std::pair<VarInterface *, VarInterface *> ViGetTableKv(int) const override { return {}; }
    [[nodiscard]] std::string ViToString(int) const override { return "test"; }
};

TEST(state, set_get_var_interface_new_func) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    int call_count = 0;
    SetVarInterfaceNewFunc(s, [&call_count]() -> VarInterface * {
        ++call_count;
        return new TestVarImpl();
    });

    auto &fn = GetVarInterfaceNewFunc(s);
    ASSERT_TRUE(fn != nullptr);

    // Calling the stored factory increments the counter
    VarInterface *vi = fn();
    ASSERT_NE(vi, nullptr);
    ASSERT_EQ(call_count, 1);
    delete vi;
}

// ---------------------------------------------------------------------------
// CompileConfig::skip_jit path
// ---------------------------------------------------------------------------

TEST(state, compile_config_skip_jit) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    CompileConfig cfg;
    cfg.skip_jit = true;

    // Compilation should succeed without registering any JIT function.
    ASSERT_NO_THROW(CompileString(s, "function test() return 1 end", cfg));

    // No function should have been registered because JIT was skipped.
    int arg_count = -1;
    void *addr = inter::GetFuncAddr(s, JIT_TCC, "test", arg_count);
    ASSERT_EQ(addr, nullptr);
}

// ---------------------------------------------------------------------------
// CompileConfig::record_c_code with live TCC JIT
// ---------------------------------------------------------------------------

TEST(state, compile_config_record_c_code_with_live_jit) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    CompileConfig cfg;
    cfg.record_c_code = true;
    cfg.debug_mode = false;
    // Leave both JIT backends at their defaults (enabled) so that the
    // recorded_c_code path is exercised with a real TCC compilation.
    cfg.disable_jit[JIT_TCC] = false;
    cfg.disable_jit[JIT_GCC] = true; // only TCC so the test is fast

    ASSERT_NO_THROW(CompileString(s, "function test() return 99 end", cfg));

    const std::string code = GetLastRecordedCCode(s);
    ASSERT_FALSE(code.empty());
    // The recorded code should contain the function name.
    ASSERT_NE(code.find("test"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Vm::AllocGlobalName counter
// ---------------------------------------------------------------------------

TEST(state, vm_alloc_global_name_counter) {
    State s;
    const std::string name0 = s.GetVM().AllocGlobalName();
    const std::string name1 = s.GetVM().AllocGlobalName();
    const std::string name2 = s.GetVM().AllocGlobalName();

    ASSERT_EQ(name0, "__fakelua_global_0__");
    ASSERT_EQ(name1, "__fakelua_global_1__");
    ASSERT_EQ(name2, "__fakelua_global_2__");
    // Names must all be distinct
    ASSERT_NE(name0, name1);
    ASSERT_NE(name1, name2);
}

// ---------------------------------------------------------------------------
// VmFunction accessors and Merge
// ---------------------------------------------------------------------------

static CVar vmf_a() {
    Var r;
    r.SetInt(10);
    return r;
}
static CVar vmf_b() {
    Var r;
    r.SetInt(20);
    return r;
}

TEST(state, vm_function_empty) {
    VmFunction empty_func;
    ASSERT_TRUE(empty_func.Empty());

    VmFunction named_func("myfn", 3, JIT_TCC, reinterpret_cast<void *>(&vmf_a), {});
    ASSERT_FALSE(named_func.Empty());
}

TEST(state, vm_function_accessors) {
    VmFunction f("myfn", 2, JIT_TCC, reinterpret_cast<void *>(&vmf_a), {});
    ASSERT_EQ(f.GetName(), "myfn");
    ASSERT_EQ(f.GetArgCount(), 2);
    ASSERT_EQ(f.GetAddr(JIT_TCC), reinterpret_cast<void *>(&vmf_a));
    ASSERT_EQ(f.GetAddr(JIT_GCC), nullptr);
    // Handle for TCC should be null (we passed {})
    ASSERT_EQ(f.GetHandle(JIT_TCC), nullptr);
}

TEST(state, vm_function_merge) {
    // Start with a TCC-only function
    VmFunction f("merge_fn", 0, JIT_TCC, reinterpret_cast<void *>(&vmf_a), {});
    ASSERT_EQ(f.GetAddr(JIT_TCC), reinterpret_cast<void *>(&vmf_a));
    ASSERT_EQ(f.GetAddr(JIT_GCC), nullptr);

    // Merge in a GCC-typed function carrying vmf_b address
    VmFunction gcc_func("merge_fn", 0, JIT_GCC, reinterpret_cast<void *>(&vmf_b), {});
    f.Merge(gcc_func);

    // After merge TCC address is unchanged, GCC address is now set
    ASSERT_EQ(f.GetAddr(JIT_TCC), reinterpret_cast<void *>(&vmf_a));
    ASSERT_EQ(f.GetAddr(JIT_GCC), reinterpret_cast<void *>(&vmf_b));
}

TEST(state, vm_register_and_get_function) {
    State s;
    ASSERT_TRUE(s.GetVM().GetFunction("absent").Empty());

    s.GetVM().RegisterFunction(VmFunction("fn_reg", 1, JIT_TCC, reinterpret_cast<void *>(&vmf_a), {}));
    VmFunction result = s.GetVM().GetFunction("fn_reg");
    ASSERT_FALSE(result.Empty());
    ASSERT_EQ(result.GetName(), "fn_reg");
    ASSERT_EQ(result.GetArgCount(), 1);
}
