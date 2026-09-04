#include "native/random/native_random.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "native/object/native_object.h"
#include "var/var.h"

#include <cstdint>
#include <cstdio>
#include <limits>
#include <numeric>
#include <vector>

namespace fakelua::random {

// ─────────────────────────────────────────────────────────────────────────────
// PCG-32 随机数生成器
// 64-bit 状态，32-bit 输出，周期 2^64，适合游戏场景
// ─────────────────────────────────────────────────────────────────────────────

static constexpr uint64_t kPCGMultiplier = 6364136223846793005ULL;
static constexpr uint64_t kPCGIncrement = 1442695040888963407ULL;

// 从 RNG 对象读取 64-bit 状态
static uint64_t rng_get_state(const NativeObject *obj) {
    return static_cast<uint64_t>(obj->GetInt("__state", 0));
}

// 写入 RNG 对象的 64-bit 状态
static void rng_set_state(NativeObject *obj, uint64_t state) {
    obj->SetInt("__state", static_cast<int64_t>(state));
}

// PCG 单步推进
static uint64_t pcg_advance(uint64_t state) {
    return state * kPCGMultiplier + kPCGIncrement;
}

// PCG-XSH-RR 输出函数：64-bit 状态 → 32-bit 输出
static uint32_t pcg_output(uint64_t state) {
    uint32_t xorshifted = static_cast<uint32_t>(((state >> 18) ^ state) >> 27);
    uint32_t rot = static_cast<uint32_t>(state >> 59);
    return (xorshifted >> rot) | (xorshifted << ((-static_cast<int32_t>(rot)) & 31));
}

// splitmix64：将任意 seed 扩展为 64-bit PCG 初始状态
static uint64_t splitmix64(uint64_t &x) {
    uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

// 生成下一个 uint32 并推进状态
static uint32_t rng_next_uint32(NativeObject *obj) {
    uint64_t state = rng_get_state(obj);
    uint32_t result = pcg_output(state);
    rng_set_state(obj, pcg_advance(state));
    return result;
}

// 生成 [0, 1) 双精度浮点
static double rng_next_unit(NativeObject *obj) {
    // 用 53-bit 精度（与 Lua math.random 一致）
    uint32_t hi = rng_next_uint32(obj);
    uint32_t lo = rng_next_uint32(obj);
    uint64_t val = (static_cast<uint64_t>(hi) << 21) | (static_cast<uint64_t>(lo) >> 11);
    return static_cast<double>(val) / static_cast<double>(1ULL << 53);
}

// ─────────────────────────────────────────────────────────────────────────────
// 方法实现
// ─────────────────────────────────────────────────────────────────────────────

// rng:int(min, max) — 整数均匀分布 [min, max]
static CVar rng_int(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "rng:int", "min and max expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    int64_t lo = inter::CVarToInteger(a0, 0);
    int64_t hi = inter::CVarToInteger(a1, 0);

    if (lo > hi) ThrowBadArgument(2, "rng:int", "max must be >= min");

    // 处理全范围情况，避免溢出
    if (lo == std::numeric_limits<int64_t>::min() && hi == std::numeric_limits<int64_t>::max()) {
        // 使用 64-bit 均匀分布
        uint32_t hi32 = rng_next_uint32(self);
        uint32_t lo32 = rng_next_uint32(self);
        uint64_t val = (static_cast<uint64_t>(hi32) << 32) | lo32;
        return inter::NativeToFakeluaLonglong(s, static_cast<long long>(val));
    }

    uint64_t range = static_cast<uint64_t>(hi) - static_cast<uint64_t>(lo) + 1;

    // 拒绝采样：避免模偏差
    uint64_t threshold = (std::numeric_limits<uint64_t>::max() / range) * range;
    uint64_t r;
    do {
        uint32_t hi32 = rng_next_uint32(self);
        uint32_t lo32 = rng_next_uint32(self);
        r = (static_cast<uint64_t>(hi32) << 32) | lo32;
    } while (r >= threshold);

    int64_t result = static_cast<int64_t>(static_cast<uint64_t>(lo) + (r % range));
    return inter::NativeToFakeluaLonglong(s, result);
}

// rng:float(min, max) — 浮点均匀分布 [min, max)
static CVar rng_float(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "rng:float", "min and max expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    double lo = inter::CVarToNumber(a0, 0.0);
    double hi = inter::CVarToNumber(a1, 0.0);

    if (lo >= hi) ThrowBadArgument(2, "rng:float", "max must be > min");

    double val = lo + rng_next_unit(self) * (hi - lo);
    return inter::NativeToFakeluaDouble(s, val);
}

// rng:dice(count, sides) — 掷 count 个 sides 面骰子，返回总和
static CVar rng_dice(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "rng:dice", "count and sides expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    CVar a1 = inter::GetNativeArg(s, args, n, 1);
    int64_t count = inter::CVarToInteger(a0, 0);
    int64_t sides = inter::CVarToInteger(a1, 0);

    if (count <= 0) ThrowBadArgument(1, "rng:dice", "count must be positive");
    if (sides <= 0) ThrowBadArgument(2, "rng:dice", "sides must be positive");
    if (count > 100000) ThrowBadArgument(1, "rng:dice", "count too large");
    if (sides > std::numeric_limits<int64_t>::max() / count) {
        ThrowBadArgument(2, "rng:dice", "dice sum would overflow");
    }

    int64_t sum = 0;
    for (int64_t i = 0; i < count; i++) {
        // 拒绝采样：生成 [1, sides] 的均匀整数
        uint64_t range = static_cast<uint64_t>(sides);
        uint64_t threshold = (std::numeric_limits<uint64_t>::max() / range) * range;
        uint64_t r;
        do {
            uint32_t hi32 = rng_next_uint32(self);
            uint32_t lo32 = rng_next_uint32(self);
            r = (static_cast<uint64_t>(hi32) << 32) | lo32;
        } while (r >= threshold);
        sum += static_cast<int64_t>(r % range) + 1;
    }

    return inter::NativeToFakeluaInt(s, sum);
}

// rng:chance(prob) — 以 prob 概率返回 true
static CVar rng_chance(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "rng:chance", "probability expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    double prob = inter::CVarToNumber(a0, 0.0);

    if (prob <= 0.0) return inter::NativeToFakeluaBool(s, false);
    if (prob >= 1.0) return inter::NativeToFakeluaBool(s, true);

    double roll = rng_next_unit(self);
    return inter::NativeToFakeluaBool(s, roll < prob);
}

// rng:weighted(weights) — 按权重表选取，返回 1-based 下标
static CVar rng_weighted(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "rng:weighted", "weights table expected");

    CVar tbl = inter::GetNativeArg(s, args, n, 0);
    if (tbl.type_ != static_cast<int>(VarType::Table)) {
        ThrowBadArgument(1, "rng:weighted", "weights must be a table");
    }

    int64_t len = table::TableHelper::GetTableLen(tbl);
    if (len <= 0) return inter::NativeToFakeluaNil(s);

    // 收集权重并计算总和
    std::vector<double> weights;
    weights.reserve(static_cast<size_t>(len));
    double total = 0.0;
    for (int64_t i = 1; i <= len; i++) {
        CVar val = table::TableHelper::GetTableInt(s, tbl, i);
        double w = inter::CVarToNumber(val, 0.0);
        if (w < 0.0) w = 0.0;
        weights.push_back(w);
        total += w;
    }

    if (total <= 0.0) return inter::NativeToFakeluaNil(s);

    // 随机选取
    double roll = rng_next_unit(self) * total;
    double cumulative = 0.0;
    for (int64_t i = 0; i < len; i++) {
        cumulative += weights[static_cast<size_t>(i)];
        if (roll < cumulative) {
            return inter::NativeToFakeluaInt(s, i + 1);  // 1-based
        }
    }

    // 浮点精度兜底
    return inter::NativeToFakeluaInt(s, len);
}

// rng:get_state() — 获取 64-bit 内部状态字符串（用于存档）
// 返回 16 位十六进制字符串，如 "0x1234567890ABCDEF"
static CVar rng_get_state_method(NativeObject *self, State *s, CVar * /*args*/, int /*n*/) {
    uint64_t state = rng_get_state(self);
    char buf[20];
    // Use ::snprintf to avoid std::snprintf not being declared on MinGW
    ::snprintf(buf, sizeof(buf), "0x%016llX", static_cast<unsigned long long>(state));
    return inter::NativeToFakeluaString(s, std::string(buf));
}

// rng:set_state(state) — 从十六进制字符串恢复 64-bit 内部状态（用于读档）
static CVar rng_set_state_method(NativeObject *self, State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "rng:set_state", "state expected");

    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string hex_str;
    if (a0.type_ == static_cast<int>(VarType::String) && a0.data_.s) {
        hex_str = std::string(a0.data_.s->Str());
    } else if (a0.type_ == static_cast<int>(VarType::StringId) && a0.data_.i) {
        const char *ptr = reinterpret_cast<const char *>(a0.data_.i);
        int sz = *reinterpret_cast<const int *>(ptr);
        hex_str.assign(ptr + 8, sz);
    } else {
        ThrowBadArgument(1, "rng:set_state", "state must be a hex string");
    }

    // 解析十六进制字符串
    uint64_t state = 0;
    size_t pos = 0;
    if (hex_str.size() >= 2 && hex_str[0] == '0' && (hex_str[1] == 'x' || hex_str[1] == 'X')) {
        pos = 2;
    }
    for (; pos < hex_str.size(); pos++) {
        char c = hex_str[pos];
        state <<= 4;
        if (c >= '0' && c <= '9') state |= static_cast<uint64_t>(c - '0');
        else if (c >= 'a' && c <= 'f') state |= static_cast<uint64_t>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') state |= static_cast<uint64_t>(c - 'A' + 10);
        else break;
    }

    rng_set_state(self, state);
    return inter::NativeToFakeluaNil(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// 注册
// ─────────────────────────────────────────────────────────────────────────────

void RegisterRandomLibraryApi(State *s) {
    if (!s) return;

    // random.new(seed) — 创建新的 RNG 实例
    RegisterNativeFunction(s, "random.new", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        int64_t seed = inter::CVarToInteger(a0, 0);

        // 使用 splitmix64 将任意 seed 扩展为 64-bit PCG 状态
        uint64_t seed_state = static_cast<uint64_t>(seed);
        uint64_t init_state = splitmix64(seed_state);

        // 创建 RNG 对象
        NativeObject *obj = NativeObjectManager::Instance().Create(
            NativeObjectManager::Instance().CreateGroup(), "rng", 0);
        rng_set_state(obj, init_state);

        // 注册方法
        obj->RegisterMethod("int", rng_int);
        obj->RegisterMethod("float", rng_float);
        obj->RegisterMethod("dice", rng_dice);
        obj->RegisterMethod("chance", rng_chance);
        obj->RegisterMethod("weighted", rng_weighted);
        obj->RegisterMethod("get_state", rng_get_state_method);
        obj->RegisterMethod("set_state", rng_set_state_method);

        return obj->Wrap(state);
    });
}

}  // namespace fakelua::random
