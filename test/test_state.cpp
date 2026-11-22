#include "compile/compiler.h"
#include "fakelua.h"
#include "jit/vm.h"
#include "jit/vm_function.h"
#include "state/const_string.h"
#include "state/state.h"
#include "test_jit.h"
#include "var/var.h"
#include "var/var_string.h"
#include "gtest/gtest.h"
#include <cmath>
#include <limits>

using namespace fakelua;

// ConstString tests

TEST(state, const_string_alloc_and_interning) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();
    auto &cs = s->GetConstString();

    const int64_t id1 = cs.Alloc("hello");
    const int64_t id2 = cs.Alloc("world");
    const int64_t id3 = cs.Alloc("hello");// should return same id as id1

    ASSERT_NE(id1, id2);
    ASSERT_EQ(id1, id3);// interning: same string → same ID
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
    const int64_t id = cs.Alloc("unique_key_abc");
    ASSERT_EQ(cs.Size(), before + 1u);

    // Reallocating same string must not increase size
    const int64_t id2 = cs.Alloc("unique_key_abc");
    ASSERT_EQ(id2, id);
    ASSERT_EQ(cs.Size(), before + 1u);
}

// GetFuncAddr tests

static CVar DummyFn(VarClosure *_CL) {
    Var r;
    r.SetInt(42);
    return r;
}

TEST(state, get_func_addr_success) {
    State s;
    s.GetVM().RegisterFunction(VmFunction("myfunc", 0, JIT_TCC, reinterpret_cast<void *>(&DummyFn), {}));

    int arg_count = -1;
    bool dummy_vararg = false;
    void *addr = inter::GetFuncAddr(&s, JIT_TCC, "myfunc", arg_count, dummy_vararg);
    ASSERT_NE(addr, nullptr);
    ASSERT_EQ(arg_count, 0);
    ASSERT_EQ(addr, reinterpret_cast<void *>(&DummyFn));
}

TEST(state, get_func_addr_missing_returns_null) {
    State s;
    int arg_count = -1;
    bool dummy_vararg = false;
    void *addr = inter::GetFuncAddr(&s, JIT_TCC, "nonexistent", arg_count, dummy_vararg);
    ASSERT_EQ(addr, nullptr);
    // arg_count must remain unchanged when function is not found
    ASSERT_EQ(arg_count, -1);
}

// Reentrant count tests

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

// inter::Reset tests

TEST(state, reset_clears_heap) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    // Allocate some temp memory so that the heap is non-empty.
    auto &temp_alloc = s->GetHeap().GetAllocator(false);
    ASSERT_NE(temp_alloc.Alloc(256), nullptr);
    ASSERT_GT(temp_alloc.Size(), 0u);

    inter::Reset(s);
    ASSERT_EQ(temp_alloc.Size(), 0u);
}

// SetVarInterfaceNewFunc / GetVarInterfaceNewFunc tests

struct TestVarImpl final : public VarInterface {
    [[nodiscard]] Type ViGetType() const override {
        return Type::NIL;
    }

    void ViSetNil() override {
    }

    void ViSetBool(bool) override {
    }

    void ViSetInt(int64_t) override {
    }

    void ViSetFloat(double) override {
    }

    void ViSetString(const std::string_view &) override {
    }

    void ViSetTable(const std::vector<std::pair<VarInterface *, VarInterface *>> &) override {
    }

    [[nodiscard]] bool ViGetBool() const override {
        return false;
    }

    [[nodiscard]] int64_t ViGetInt() const override {
        return 0;
    }

    [[nodiscard]] double ViGetFloat() const override {
        return 0;
    }

    [[nodiscard]] std::string_view ViGetString() const override {
        return {};
    }

    [[nodiscard]] size_t ViGetTableSize() const override {
        return 0;
    }

    [[nodiscard]] std::pair<VarInterface *, VarInterface *> ViGetTableKv(int) const override {
        return {};
    }

    [[nodiscard]] std::string ViToString(int) const override {
        return "test";
    }
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

// CompileConfig::skip_jit path

TEST(state, compile_config_skip_jit) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    CompileConfig cfg;
    cfg.skip_jit = true;

    // Compilation should succeed without registering any JIT function.
    ASSERT_NO_THROW(CompileString(s, "function test() return 1 end", cfg));

    // No function should have been registered because JIT was skipped.
    int arg_count = -1;
    bool dummy_vararg = false;
    void *addr = inter::GetFuncAddr(s, JIT_TCC, "test", arg_count, dummy_vararg);
    ASSERT_EQ(addr, nullptr);
}

// CompileConfig::record_c_code with live TCC JIT

TEST(state, compile_config_record_c_code_with_live_jit) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    CompileConfig cfg;
    cfg.record_c_code = true;
    cfg.debug_mode = false;
    // Leave both JIT backends at their defaults (enabled) so that the
    // recorded_c_code path is exercised with a real TCC compilation.
    cfg.disable_jit[JIT_TCC] = false;
    cfg.disable_jit[JIT_GCC] = true;// only TCC so the test is fast

    ASSERT_NO_THROW(CompileString(s, "function test() return 99 end", cfg));

    const std::string code = GetLastRecordedCCode(s);
    ASSERT_FALSE(code.empty());
    // The recorded code should contain the function name.
    ASSERT_NE(code.find("test"), std::string::npos);
}

// Vm::AllocGlobalName counter

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

// VmFunction accessors and Merge

static CVar VmfA(VarClosure *_CL) {
    Var r;
    r.SetInt(10);
    return r;
}

static CVar VmfB(VarClosure *_CL) {
    Var r;
    r.SetInt(20);
    return r;
}

TEST(state, vm_function_empty) {
    VmFunction empty_func;
    ASSERT_TRUE(empty_func.Empty());

    VmFunction named_func("myfn", 3, JIT_TCC, reinterpret_cast<void *>(&VmfA), {});
    ASSERT_FALSE(named_func.Empty());
}

TEST(state, vm_function_accessors) {
    VmFunction f("myfn", 2, JIT_TCC, reinterpret_cast<void *>(&VmfA), {});
    ASSERT_EQ(f.GetName(), "myfn");
    ASSERT_EQ(f.GetArgCount(), 2);
    ASSERT_EQ(f.GetAddr(JIT_TCC), reinterpret_cast<void *>(&VmfA));
    ASSERT_EQ(f.GetAddr(JIT_GCC), nullptr);
    // Handle for TCC should be null (we passed {})
    ASSERT_EQ(f.GetHandle(JIT_TCC), nullptr);
}

TEST(state, vm_function_merge) {
    // Start with a TCC-only function
    VmFunction f("merge_fn", 0, JIT_TCC, reinterpret_cast<void *>(&VmfA), {});
    ASSERT_EQ(f.GetAddr(JIT_TCC), reinterpret_cast<void *>(&VmfA));
    ASSERT_EQ(f.GetAddr(JIT_GCC), nullptr);

    // Merge in a GCC-typed function carrying VmfB address
    VmFunction gcc_func("merge_fn", 0, JIT_GCC, reinterpret_cast<void *>(&VmfB), {});
    f.Merge(gcc_func);

    // After merge TCC address is unchanged, GCC address is now set
    ASSERT_EQ(f.GetAddr(JIT_TCC), reinterpret_cast<void *>(&VmfA));
    ASSERT_EQ(f.GetAddr(JIT_GCC), reinterpret_cast<void *>(&VmfB));
}

TEST(state, vm_function_merge_updates_arity) {
    VmFunction f("merge_fn", 0, JIT_TCC, reinterpret_cast<void *>(&VmfA), {}, false);
    ASSERT_EQ(f.GetArgCount(), 0);
    ASSERT_FALSE(f.IsVararg());

    VmFunction vararg_func("merge_fn", 1, JIT_GCC, reinterpret_cast<void *>(&VmfB), {}, true);
    f.Merge(vararg_func);

    ASSERT_EQ(f.GetArgCount(), 1);
    ASSERT_TRUE(f.IsVararg());
    ASSERT_EQ(f.GetAddr(JIT_TCC), reinterpret_cast<void *>(&VmfA));
    ASSERT_EQ(f.GetAddr(JIT_GCC), reinterpret_cast<void *>(&VmfB));
}

TEST(state, recompile_vararg_call) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();
    ASSERT_NO_THROW(CompileString(s, "function f() return 0 end", {}));
    ASSERT_NO_THROW(CompileString(s, "function f(...) return select('#', ...) end", {}));
    int64_t ret = 0;
    CallAll(s, "f", ret, 7);
    ASSERT_EQ(ret, 1);
    CallAll(s, "f", ret, 1, 2, 3);
    ASSERT_EQ(ret, 3);
}

TEST(state, vm_register_and_get_function) {
    State s;
    ASSERT_TRUE(s.GetVM().GetFunction("absent").Empty());

    s.GetVM().RegisterFunction(VmFunction("fn_reg", 1, JIT_TCC, reinterpret_cast<void *>(&VmfA), {}));
    VmFunction result = s.GetVM().GetFunction("fn_reg");
    ASSERT_FALSE(result.Empty());
    ASSERT_EQ(result.GetName(), "fn_reg");
    ASSERT_EQ(result.GetArgCount(), 1);
}

// ConstString empty string

TEST(state, const_string_empty) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();
    auto &cs = s->GetConstString();

    const int64_t id = cs.Alloc("");
    ASSERT_NE(id, -1);// empty string must get a valid ID

    const std::string_view sv = ConstString::GetString(id);
    ASSERT_EQ(sv, "");// must retrieve the empty string

    const VarString *vs = ConstString::GetVarString(id);
    ASSERT_NE(vs, nullptr);
    ASSERT_EQ(vs->Str(), "");

    // Allocating empty again must return the SAME id (interning)
    const int64_t id2 = cs.Alloc("");
    ASSERT_EQ(id2, id);
}

// ConstString large key

TEST(state, const_string_large) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();
    auto &cs = s->GetConstString();

    // Build a 2KB string
    std::string large(2048, 'X');
    for (size_t i = 0; i < large.size(); ++i) {
        large[i] = static_cast<char>('A' + (i % 26));
    }

    const int64_t id = cs.Alloc(large);
    const std::string_view sv = ConstString::GetString(id);
    ASSERT_EQ(sv, large);
    ASSERT_EQ(sv.size(), 2048u);
}

// State reset and reuse

TEST(state, reset_reuse_state) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    // Phase 1: compile and call a function
    ASSERT_NO_THROW(CompileString(s, "function first() return 111 end", {}));
    {
        int64_t ret = 0;
        CallAll(s, "first", ret);
        ASSERT_EQ(ret, 111);
    }

    // Reset the state
    inter::Reset(s);

    // Phase 2: compile and call a DIFFERENT function
    ASSERT_NO_THROW(CompileString(s, "function second() return 222 end", {}));
    {
        int64_t ret = 0;
        CallAll(s, "second", ret);
        ASSERT_EQ(ret, 222);
    }

    // Phase 3: a second Reset and yet another compilation should work
    inter::Reset(s);
    ASSERT_NO_THROW(CompileString(s, "function third() return 333 end", {}));
    {
        int64_t ret = 0;
        CallAll(s, "third", ret);
        ASSERT_EQ(ret, 333);
    }
}

// Heap allocator: size 0 and block boundary

TEST(state, heap_alloc_boundary) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();
    auto &alloc = s->GetHeap().GetAllocator(false);

    // Alloc of size 0 should succeed (implementation may return a valid pointer or nullptr)
    (void) alloc.Alloc(0);// result may be nullptr; verify no crash
    // After alloc(0), subsequent operations must still work
    void *p1 = alloc.Alloc(64);
    ASSERT_NE(p1, nullptr);

    // Reset the allocator
    inter::Reset(s);

    // Allocate exactly 1 byte
    void *pb = alloc.Alloc(1);
    ASSERT_NE(pb, nullptr);

    // Reset and verify empty
    inter::Reset(s);
    ASSERT_EQ(alloc.Size(), 0u);
}

TEST(state, cvar_to_integer_edge_cases) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    CVar i{};
    i.type_ = static_cast<int>(VarType::Int);
    i.data_.i = 42;
    EXPECT_EQ(inter::CVarToInteger(i, -1), 42);

    CVar btrue{};
    btrue.type_ = static_cast<int>(VarType::Bool);
    btrue.data_.b = true;
    CVar bfalse{};
    bfalse.type_ = static_cast<int>(VarType::Bool);
    bfalse.data_.b = false;
    EXPECT_EQ(inter::CVarToInteger(btrue, -1), 1);
    EXPECT_EQ(inter::CVarToInteger(bfalse, -1), 0);

    CVar f{};
    f.type_ = static_cast<int>(VarType::Float);
    f.data_.f = 3.9;
    EXPECT_EQ(inter::CVarToInteger(f, -1), 3);

    CVar nanv{};
    nanv.type_ = static_cast<int>(VarType::Float);
    nanv.data_.f = std::numeric_limits<double>::quiet_NaN();
    EXPECT_EQ(inter::CVarToInteger(nanv, 99), 99);

    CVar inf{};
    inf.type_ = static_cast<int>(VarType::Float);
    inf.data_.f = std::numeric_limits<double>::infinity();
    EXPECT_EQ(inter::CVarToInteger(inf, 99), 99);

    CVar huge{};
    huge.type_ = static_cast<int>(VarType::Float);
    huge.data_.f = 1e300;
    EXPECT_EQ(inter::CVarToInteger(huge, 99), 99);

    Var vs;
    vs.SetTempString(s, "123");
    EXPECT_EQ(inter::CVarToInteger(reinterpret_cast<const CVar &>(vs), -1), 123);

    Var vsf;
    vsf.SetTempString(s, "12.5");
    EXPECT_EQ(inter::CVarToInteger(reinterpret_cast<const CVar &>(vsf), -1), 12);

    Var vsbad;
    vsbad.SetTempString(s, "12x");
    EXPECT_EQ(inter::CVarToInteger(reinterpret_cast<const CVar &>(vsbad), 77), 77);

    Var vempty;
    vempty.SetTempString(s, "");
    EXPECT_EQ(inter::CVarToInteger(reinterpret_cast<const CVar &>(vempty), 77), 77);

    Var vhuge;
    vhuge.SetTempString(s, "1e300");
    EXPECT_EQ(inter::CVarToInteger(reinterpret_cast<const CVar &>(vhuge), 77), 77);

    CVar nil{};
    EXPECT_EQ(inter::CVarToInteger(nil, 55), 55);
}

TEST(state, cvar_to_number_bool_string) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    CVar f{};
    f.type_ = static_cast<int>(VarType::Float);
    f.data_.f = 1.5;
    EXPECT_DOUBLE_EQ(inter::CVarToNumber(f, -1), 1.5);

    CVar i{};
    i.type_ = static_cast<int>(VarType::Int);
    i.data_.i = 8;
    EXPECT_DOUBLE_EQ(inter::CVarToNumber(i, -1), 8.0);

    Var vs;
    vs.SetTempString(s, "2.5");
    EXPECT_DOUBLE_EQ(inter::CVarToNumber(reinterpret_cast<const CVar &>(vs), -1), 2.5);

    Var vsbad;
    vsbad.SetTempString(s, "nope");
    EXPECT_DOUBLE_EQ(inter::CVarToNumber(reinterpret_cast<const CVar &>(vsbad), 9.0), 9.0);

    CVar zero{};
    zero.type_ = static_cast<int>(VarType::Int);
    zero.data_.i = 0;
    EXPECT_TRUE(inter::CVarToBool(zero, false));

    CVar nil{};
    EXPECT_TRUE(inter::CVarToBool(nil, true));
    EXPECT_FALSE(inter::CVarToBool(nil, false));

    CVar bfalse{};
    bfalse.type_ = static_cast<int>(VarType::Bool);
    bfalse.data_.b = false;
    EXPECT_FALSE(inter::CVarToBool(bfalse, true));

    Var hello;
    hello.SetTempString(s, "hi");
    EXPECT_EQ(inter::CVarToString(reinterpret_cast<const CVar &>(hello), "x"), "hi");
    EXPECT_EQ(inter::CVarToString(CVar{}, "def"), "def");
    EXPECT_EQ(inter::CVarToString(CVar{}, nullptr), "");
}

TEST(state, two_states_same_thread_same_function_names) {
    FakeluaStateGuard guard1;
    FakeluaStateGuard guard2;
    auto *s1 = guard1.GetState();
    auto *s2 = guard2.GetState();

    // 同一线程上的两个 State：注册同名但实现不同的原生函数组
    RegisterNativeFunction(s1, "host_tag", false, std::function<int64_t(State *, int64_t)>([](State *, int64_t x) -> int64_t { return x + 100; }));
    RegisterNativeFunction(s1, "host_combine", false, std::function<int64_t(State *, int64_t, int64_t)>([](State *, int64_t a, int64_t b) -> int64_t { return a * 10 + b; }));

    RegisterNativeFunction(s2, "host_tag", false, std::function<int64_t(State *, int64_t)>([](State *, int64_t x) -> int64_t { return x * 1000; }));
    RegisterNativeFunction(s2, "host_combine", false, std::function<int64_t(State *, int64_t, int64_t)>([](State *, int64_t a, int64_t b) -> int64_t { return a - b; }));

    const char *script1 = R"(
        local g_bias = 7

        function compute(a, b)
            return a + b + g_bias
        end

        function transform_label(name)
            return "s1:" .. name
        end

        function make_step_sum(base, n)
            local acc = base
            local function step(v)
                acc = acc + v
                return acc
            end
            for i = 1, n do
                step(i)
            end
            return acc
        end

        function tail_acc(n, acc)
            if n <= 0 then
                return acc
            end
            return tail_acc(n - 1, acc + n)
        end

        function run_suite(x, y)
            local c = compute(x, y)
            local h = host_tag(x)
            local m = make_step_sum(10, y)
            local t = tail_acc(y, 0)
            return host_combine(c, h) + m + t
        end
    )";

    const char *script2 = R"(
        local g_bias = 50

        function compute(a, b)
            return a * b + g_bias
        end

        function transform_label(name)
            return "s2[" .. name .. "]"
        end

        function make_step_sum(base, n)
            local acc = base
            local function step(v)
                acc = acc + v * 10
                return acc
            end
            for i = 1, n do
                step(i)
            end
            return acc
        end

        function tail_acc(n, acc)
            if n <= 0 then
                return acc
            end
            return tail_acc(n - 1, acc + n * 2)
        end

        function run_suite(x, y)
            local c = compute(x, y)
            local h = host_tag(x)
            local m = make_step_sum(10, y)
            local t = tail_acc(y, 0)
            return host_combine(c, h) + m + t
        end
    )";

    ASSERT_NO_THROW(CompileString(s1, script1, {}));
    ASSERT_NO_THROW(CompileString(s2, script2, {}));

    // 在同一个线程里按全后端交替调用两个 State 上的同名函数，验证完全隔离
    for (const auto jit_type: AllJitTypes()) {
        SCOPED_TRACE(::testing::Message() << "jit=" << JitTypeName(jit_type));

        int64_t r1_compute = 0;
        int64_t r2_compute = 0;
        Call(s1, jit_type, "compute", r1_compute, int64_t{3}, int64_t{4});// 3 + 4 + 7 = 14
        Call(s2, jit_type, "compute", r2_compute, int64_t{3}, int64_t{4});// 3 * 4 + 50 = 62
        EXPECT_EQ(r1_compute, 14);
        EXPECT_EQ(r2_compute, 62);

        std::string r1_str;
        std::string r2_str;
        Call(s1, jit_type, "transform_label", r1_str, std::string("alpha"));
        Call(s2, jit_type, "transform_label", r2_str, std::string("alpha"));
        EXPECT_EQ(r1_str, "s1:alpha");
        EXPECT_EQ(r2_str, "s2[alpha]");

        int64_t r1_closure = 0;
        int64_t r2_closure = 0;
        Call(s1, jit_type, "make_step_sum", r1_closure, int64_t{10}, int64_t{4});// 10 + (1+2+3+4) = 20
        Call(s2, jit_type, "make_step_sum", r2_closure, int64_t{10}, int64_t{4});// 10 + 10*(1+2+3+4) = 110
        EXPECT_EQ(r1_closure, 20);
        EXPECT_EQ(r2_closure, 110);

        int64_t r1_tail = 0;
        int64_t r2_tail = 0;
        Call(s1, jit_type, "tail_acc", r1_tail, int64_t{4}, int64_t{5});// 5 + 10 = 15
        Call(s2, jit_type, "tail_acc", r2_tail, int64_t{4}, int64_t{5});// 5 + 20 = 25
        EXPECT_EQ(r1_tail, 15);
        EXPECT_EQ(r2_tail, 25);

        // run_suite(3, 4):
        // s1: c=14, h=103, m=20, t=10 -> combine(14,103)=243 -> 243+20+10 = 273
        // s2: c=62, h=3000, m=110, t=20 -> combine(62,3000)=-2938 -> -2938+110+20 = -2808
        int64_t r1_suite = 0;
        int64_t r2_suite = 0;
        Call(s1, jit_type, "run_suite", r1_suite, int64_t{3}, int64_t{4});
        Call(s2, jit_type, "run_suite", r2_suite, int64_t{3}, int64_t{4});
        EXPECT_EQ(r1_suite, 273);
        EXPECT_EQ(r2_suite, -2808);
    }

    EXPECT_EQ(s1->GetInterpState().top, 0u);
    EXPECT_EQ(s1->GetInterpState().boxed_frames, 0);
    EXPECT_FALSE(s1->GetInterpState().tail.pending);
    EXPECT_EQ(s2->GetInterpState().top, 0u);
    EXPECT_EQ(s2->GetInterpState().boxed_frames, 0);
    EXPECT_FALSE(s2->GetInterpState().tail.pending);
}

TEST(state, two_states_same_thread_interleaved_interp_reentry) {
    FakeluaStateGuard guard1;
    FakeluaStateGuard guard2;
    auto *s1 = guard1.GetState();
    auto *s2 = guard2.GetState();

    // 验证解释器在同线程下从 State1 帧中回调进入 State2 的同名函数时，
    // 栈顶 saved、boxed_frames 和尾调用 trampoline 各自独立且互不干扰。
    RegisterNativeFunction(s1, "bridge_call", false, std::function<int64_t(State *, int64_t)>([s2](State *, int64_t x) -> int64_t {
        int64_t r2 = 0;
        Call(s2, JIT_INTERP, "outer_boxed", r2, x);
        return r2;
    }));
    RegisterNativeFunction(s2, "bridge_call", false, std::function<int64_t(State *, int64_t)>([](State *, int64_t x) -> int64_t { return x + 5; }));

    const char *script1 = R"(
        function boxed_tail(v)
            local box = v * 2
            local function read_box()
                return box
            end
            local sub = bridge_call(v)
            return read_box() + sub
        end

        function outer_boxed(x)
            return boxed_tail(x + 1)
        end
    )";

    const char *script2 = R"(
        function boxed_tail(v)
            local box = v * 10
            local function read_box()
                return box
            end
            return read_box() + bridge_call(v)
        end

        function outer_boxed(x)
            return boxed_tail(x * 3)
        end
    )";

    ASSERT_NO_THROW(CompileString(s1, script1, {}));
    ASSERT_NO_THROW(CompileString(s2, script2, {}));

    int64_t result = 0;
    // x = 4 -> s1.outer_boxed(4) -> s1.boxed_tail(5): box=10, bridge_call(5)
    //   -> s2.outer_boxed(5) -> s2.boxed_tail(15): box=150, s2.bridge_call(15)=20 -> 170
    // -> back to s1.boxed_tail(5): read_box()=10 + 170 = 180
    Call(s1, JIT_INTERP, "outer_boxed", result, int64_t{4});
    EXPECT_EQ(result, 180);

    // 再单独直接调用 s2 的同名入口验证
    int64_t direct_s2 = 0;
    Call(s2, JIT_INTERP, "outer_boxed", direct_s2, int64_t{4});// boxed_tail(12): 120 + 17 = 137
    EXPECT_EQ(direct_s2, 137);

    EXPECT_EQ(s1->GetInterpState().top, 0u);
    EXPECT_EQ(s1->GetInterpState().boxed_frames, 0);
    EXPECT_FALSE(s1->GetInterpState().tail.pending);
    EXPECT_EQ(s2->GetInterpState().top, 0u);
    EXPECT_EQ(s2->GetInterpState().boxed_frames, 0);
    EXPECT_FALSE(s2->GetInterpState().tail.pending);
}
