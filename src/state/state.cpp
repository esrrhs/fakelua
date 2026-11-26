#include "state/state.h"
#include <algorithm>
#include <cstring>
#include "fakelua.h"
#include "jit/tcc_jit.h"
#include "native/compress/native_compress.h"
#include "native/container/native_container.h"
#include "native/crypto/native_crypto.h"
#include "native/csv/native_csv.h"
#include "native/event/native_event.h"
#include "native/http/native_http.h"
#include "native/ini/native_ini.h"
#include "native/io/native_io.h"
#include "native/json/native_json.h"
#include "native/log/native_log.h"
#include "native/mysql/native_mysql.h"
#include "native/native_io_context.h"
#include "native/net/native_net.h"
#include "native/os/native_os.h"
#include "native/process/native_process.h"
#include "native/protobuf/native_protobuf.h"
#include "native/random/native_random.h"
#include "native/redis/native_redis.h"
#include "native/runtime/native_runtime.h"
#include "native/serialize/native_serialize.h"
#include "native/sqlite/native_sqlite.h"
#include "native/timer/native_timer.h"
#include "native/toml/native_toml.h"
#include "native/url/native_url.h"
#include "native/utf8/native_utf8.h"
#include "native/xml/native_xml.h"
#include "native/yaml/native_yaml.h"
#include "util/logging.h"
#include "util/utf8_io.h"

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#include <execinfo.h>
#endif

namespace fakelua {

// 在这里而不是头文件里：io_context_ 用的是不完整类型。
State::~State() = default;

native::IoContext &State::GetIoContext() {
    if (!io_context_) {
        io_context_ = std::make_unique<native::IoContext>();
    }
    return *io_context_;
}

void State::PushLuaFrame(const char *func, const char *file, int line, LuaFrame *saved_overflow) {
    if (lua_depth_ < 0) {
        lua_depth_ = 0;
    }
    const int idx = lua_depth_ < kMaxLuaStack ? lua_depth_ : kMaxLuaStack;
    LuaFrame &fr = lua_frames_[idx];
    if (lua_depth_ >= kMaxLuaStack && saved_overflow != nullptr) {
        *saved_overflow = fr;
    }
    fr.file = file;
    fr.line = line;
    fr.func = (func != nullptr && func[0] != '\0') ? func : "?";
    ++lua_depth_;
}

void State::PopLuaFrame(const LuaFrame *saved_overflow) {
    if (lua_depth_ > 0) {
        --lua_depth_;
        if (lua_depth_ >= kMaxLuaStack && saved_overflow != nullptr) {
            lua_frames_[kMaxLuaStack] = *saved_overflow;
        }
    }
}

void State::RetargetTopLuaFrame(const char *func, const char *file) {
    if (lua_depth_ <= 0) {
        PushLuaFrame(func, file, 0);
        return;
    }
    const int idx = (lua_depth_ - 1) < kMaxLuaStack ? (lua_depth_ - 1) : kMaxLuaStack;
    LuaFrame &fr = lua_frames_[idx];
    fr.func = (func != nullptr && func[0] != '\0') ? func : "?";
    fr.file = file;
    fr.line = 0;
}

void State::SetTopLuaLine(int line) {
    if (lua_depth_ <= 0 || line <= 0) {
        return;
    }
    const int idx = (lua_depth_ - 1) < kMaxLuaStack ? (lua_depth_ - 1) : kMaxLuaStack;
    lua_frames_[idx].line = line;
}

void State::RegisterJitFn(const void *owner, const void *addr, const char *lua_name, const char *file, int line, bool dispatcher) {
    if (addr == nullptr || lua_name == nullptr || lua_name[0] == '\0') {
        return;
    }
    JitFnSymbol sym;
    sym.owner = owner;
    sym.lua_name = lua_name;
    sym.file = file != nullptr ? file : "";
    sym.line = line;
    sym.dispatcher = dispatcher;
    jit_fn_symbols_[addr] = std::move(sym);
}

extern "C" void FakeluaRegisterFnMap(State *s, const void *owner, const FlLuaFnMap *map, int count, const char *file) {
    if (s == nullptr || map == nullptr || count <= 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        s->RegisterJitFn(owner, map[i].addr, map[i].lua_name, file, map[i].line, map[i].dispatcher != 0);
    }
}

void State::CollectMergedLuaFrames(std::vector<MergedLuaFrame> &out, int &omitted_count, int &omitted_pos) const {
    out.clear();
    omitted_count = 0;
    omitted_pos = -1;

    std::vector<const LuaFrame *> lua_recent;
    int lua_omitted = 0;
    if (lua_depth_ > 0) {
        if (lua_depth_ <= kMaxLuaStack) {
            lua_recent.reserve(static_cast<size_t>(lua_depth_));
            for (int idx = lua_depth_ - 1; idx >= 0; --idx) {
                lua_recent.push_back(&lua_frames_[idx]);
            }
        } else {
            lua_recent.reserve(static_cast<size_t>(kMaxLuaStack));
            lua_recent.push_back(&lua_frames_[kMaxLuaStack]);
            lua_omitted = lua_depth_ - kMaxLuaStack;
            for (int idx = kMaxLuaStack - 2; idx >= 0; --idx) {
                lua_recent.push_back(&lua_frames_[idx]);
            }
        }
    }

    std::vector<const JitFnSymbol *> c_frames;
    if (!jit_fn_symbols_.empty()) {
        constexpr int kMaxCFrames = 1024;
        void *pcs[kMaxCFrames];
#if defined(_WIN32) && (defined(_M_X64) || defined(__x86_64__))
        const USHORT n = CaptureStackBackTrace(0, kMaxCFrames, pcs, nullptr);
        c_frames.reserve(n);
        for (USHORT i = 0; i < n; ++i) {
            const auto pc = reinterpret_cast<DWORD64>(pcs[i]);
            if (pc == 0) {
                continue;
            }
            DWORD64 image_base = 0;
            PRUNTIME_FUNCTION rf = RtlLookupFunctionEntry(pc - 1, &image_base, nullptr);
            if (rf == nullptr) {
                continue;
            }
            const auto *fn_start = reinterpret_cast<const void *>(image_base + rf->BeginAddress);
            const auto it = jit_fn_symbols_.find(fn_start);
            if (it != jit_fn_symbols_.end()) {
                c_frames.push_back(&it->second);
            }
        }
#elif !defined(_WIN32)
        const int n = backtrace(pcs, kMaxCFrames);
        c_frames.reserve(static_cast<size_t>(std::max(n, 0)));
        for (int i = 0; i < n; ++i) {
            Dl_info info{};
            if (dladdr(pcs[i], &info) == 0 || info.dli_saddr == nullptr) {
                continue;
            }
            const auto it = jit_fn_symbols_.find(info.dli_saddr);
            if (it != jit_fn_symbols_.end()) {
                c_frames.push_back(&it->second);
            }
        }
#endif
    }

    const bool hinted = CurrentJit() == JIT_GCC && spec_throw_hint_name_ != nullptr && spec_throw_hint_name_[0] != '\0';
    bool skipped_hint_body = !hinted;
    bool can_absorb_dispatcher = hinted;

    const auto append_c_frames = [&]() {
        for (const JitFnSymbol *sym: c_frames) {
            if (!skipped_hint_body && sym->lua_name == spec_throw_hint_name_) {
                skipped_hint_body = true;
                can_absorb_dispatcher = !sym->dispatcher;
                continue;
            }
            // 只合并紧挨在特化体/去优化体底下的那一层入口分发器；普通递归函数和匿名闭包的 dispatcher 为 false，不合并。
            if (sym->dispatcher && can_absorb_dispatcher && !out.empty() && out.back().func != nullptr && sym->lua_name == out.back().func &&
                (out.back().file == nullptr || sym->file == out.back().file)) {
                can_absorb_dispatcher = false;
                continue;
            }
            out.push_back(MergedLuaFrame{sym->file.c_str(), sym->line, sym->lua_name.c_str()});
            can_absorb_dispatcher = !sym->dispatcher;
        }
    };

    const auto append_lua_frames = [&]() {
        if (lua_omitted > 0 && !lua_recent.empty()) {
            omitted_count = lua_omitted;
            omitted_pos = static_cast<int>(out.size()) + 1;
        }
        for (const LuaFrame *fr: lua_recent) {
            out.push_back(MergedLuaFrame{fr->file, fr->line, fr->func});
        }
    };

    if (CurrentJit() == JIT_GCC) {
        if (hinted) {
            out.push_back(MergedLuaFrame{spec_throw_hint_file_, spec_throw_hint_line_, spec_throw_hint_name_});
        }
        append_c_frames();
        append_lua_frames();
    } else {
        append_lua_frames();
        skipped_hint_body = true;
        can_absorb_dispatcher = false;
        append_c_frames();
    }
}

std::string State::FormatMergedLuaFrames(const std::vector<MergedLuaFrame> &frames, int omitted_count, int omitted_pos) const {
    if (frames.empty()) {
        return {};
    }
    const int depth = static_cast<int>(frames.size());
    const int shown = depth < kMaxLuaStack ? depth : kMaxLuaStack;
    const int tail_hidden = depth - shown;
    std::string out = "stack traceback:\n";
    out.reserve(static_cast<size_t>(shown + 2) * 64);

    const auto append_omitted = [&](int count) {
        if (count > 0) {
            out += "\t... (";
            out += std::to_string(count);
            out += " more frames)\n";
        }
    };

    const bool mid_omitted = omitted_count > 0 && omitted_pos >= 0 && omitted_pos < shown;
    for (int i = 0; i < shown; ++i) {
        if (mid_omitted && i == omitted_pos) {
            append_omitted(omitted_count);
        }
        const MergedLuaFrame &fr = frames[static_cast<size_t>(i)];
        out += '\t';
        if (fr.file != nullptr && fr.file[0] != '\0') {
            out += fr.file;
            if (fr.line > 0) {
                out += ':';
                out += std::to_string(fr.line);
            }
            out += ": ";
        }
        out += "in function '";
        out += (fr.func != nullptr && fr.func[0] != '\0') ? fr.func : "?";
        out += "'\n";
    }
    append_omitted((mid_omitted ? 0 : omitted_count) + tail_hidden);
    return out;
}

std::string State::FormatLuaStack() const {
    std::vector<MergedLuaFrame> frames;
    int omitted_count = 0;
    int omitted_pos = -1;
    CollectMergedLuaFrames(frames, omitted_count, omitted_pos);
    return FormatMergedLuaFrames(frames, omitted_count, omitted_pos);
}

void State::CaptureLuaError(std::string &loc, std::string &lua_stack) const {
    std::vector<MergedLuaFrame> frames;
    int omitted_count = 0;
    int omitted_pos = -1;
    CollectMergedLuaFrames(frames, omitted_count, omitted_pos);
    lua_stack = FormatMergedLuaFrames(frames, omitted_count, omitted_pos);

    if (CurrentJit() == JIT_GCC) {
        if (!frames.empty() && frames[0].file != nullptr && frames[0].file[0] != '\0' && frames[0].line > 0) {
            loc = std::string(frames[0].file) + ":" + std::to_string(frames[0].line);
            return;
        }
    }
    if (const LuaFrame *fr = TopLuaFrame()) {
        if (fr->file != nullptr && fr->file[0] != '\0' && fr->line > 0) {
            loc = std::string(fr->file) + ":" + std::to_string(fr->line);
            return;
        }
        loc.clear();
        return;
    }
    const auto jit = CurrentJit();
    const LuaLoc &active = GetActiveLuaLoc(jit);
    if (active.file != nullptr && active.file[0] != '\0' && active.line > 0) {
        loc = std::string(active.file) + ":" + std::to_string(active.line);
        return;
    }
    const auto &locs = GetLuaLocs(jit);
    for (auto it = locs.rbegin(); it != locs.rend(); ++it) {
        const LuaLoc *l = *it;
        if (l != nullptr && l->file != nullptr && l->file[0] != '\0' && l->line > 0) {
            loc = std::string(l->file) + ":" + std::to_string(l->line);
            return;
        }
    }
    loc.clear();
}

void State::SetLogFile(const std::string &path, size_t max_size, size_t max_files) {
    log_sink_.reset();
    if (!path.empty()) {
        log_sink_ = decltype(log_sink_)(CreateLogSink(path, max_size, max_files), &DestroyLogSink);
    }
}

State::State(const StateConfig &config) : config_(config), compiler_(this), heap_(&temp_heap_slot_, &const_heap_slot_), const_string_(this) {
    utf8_io::Init();
    log_level_ = config_.log_level;
    if (!config_.log_file.empty()) {
        log_sink_ = decltype(log_sink_)(CreateLogSink(config_.log_file, config_.log_max_size, config_.log_max_files), &DestroyLogSink);
    }

    RegisterNativeObjectApi(this);
    net::RegisterNetLibraryApi(this);
    http::RegisterHttpLibraryApi(this);
    url::RegisterUrlLibraryApi(this);
    timer::RegisterTimerLibraryApi(this);
    runtime::RegisterRuntimeLibraryApi(this);
    serialize::RegisterSerializeLibraryApi(this);
    protobuf::RegisterProtobufLibraryApi(this);
    crypto::RegisterCryptoLibraryApi(this);
    compress::RegisterCompressLibraryApi(this);
    json::RegisterJsonLibraryApi(this);
    csv::RegisterCsvLibraryApi(this);
    sqlite::RegisterSqliteLibraryApi(this);
    mysql::RegisterMysqlLibraryApi(this);
    mysql::RegisterMysqlPoolApi(this);
    redis::RegisterRedisLibraryApi(this);
    process::RegisterProcessLibraryApi(this);
    event::RegisterEventLibraryApi(this);
    random::RegisterRandomLibraryApi(this);
    container::RegisterContainerLibraryApi(this);
    yaml::RegisterYamlLibraryApi(this);
    xml::RegisterXmlLibraryApi(this);
    toml::RegisterTomlLibraryApi(this);
    ini::RegisterIniLibraryApi(this);
    log::RegisterLogLibraryApi(this);
}

}// namespace fakelua
