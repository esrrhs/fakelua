#include "state/state.h"
#include <algorithm>
#include <cstring>
#if !defined(_WIN32)
#include <dlfcn.h>
#include <execinfo.h>
#endif
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

namespace fakelua {

// 在这里而不是头文件里：io_context_ 用的是不完整类型。
State::~State() = default;

native::IoContext &State::GetIoContext() {
    if (!io_context_) {
        io_context_ = std::make_unique<native::IoContext>();
    }
    return *io_context_;
}

void State::PushLuaFrame(const char *func, const char *file, int line) {
    if (lua_depth_ < 0) {
        lua_depth_ = 0;
    }
    LuaFrame &fr = lua_frames_[lua_depth_ % kMaxLuaStack];
    fr.file = file;
    fr.line = line;
    fr.func = (func != nullptr && func[0] != '\0') ? func : "?";
    ++lua_depth_;
}

void State::PopLuaFrame() {
    if (lua_depth_ > 0) {
        --lua_depth_;
    }
}

void State::RetargetTopLuaFrame(const char *func, const char *file) {
    if (lua_depth_ <= 0) {
        PushLuaFrame(func, file, 0);
        return;
    }
    LuaFrame &fr = lua_frames_[(lua_depth_ - 1) % kMaxLuaStack];
    fr.func = (func != nullptr && func[0] != '\0') ? func : "?";
    fr.file = file;
    fr.line = 0;
}

void State::SetTopLuaLine(int line) {
    if (lua_depth_ <= 0 || line <= 0) {
        return;
    }
    lua_frames_[(lua_depth_ - 1) % kMaxLuaStack].line = line;
}

void State::RegisterJitFn(const void *addr, const char *lua_name, const char *file, int line, bool spec) {
    if (addr == nullptr || lua_name == nullptr || lua_name[0] == '\0') {
        return;
    }
    JitFnSymbol sym;
    sym.lua_name = lua_name;
    sym.file = file != nullptr ? file : "";
    sym.line = line;
    sym.spec = spec;
    jit_fn_symbols_[addr] = std::move(sym);
}

extern "C" void FakeluaRegisterFnMap(State *s, const FlLuaFnMap *map, int count, const char *file) {
    if (s == nullptr || map == nullptr || count <= 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        s->RegisterJitFn(map[i].addr, map[i].lua_name, file, map[i].line, map[i].spec != 0);
    }
}

void State::CollectMergedLuaFrames(std::vector<MergedLuaFrame> &out) const {
    out.clear();
    std::vector<const LuaFrame *> lua_recent;
    lua_recent.reserve(static_cast<size_t>(std::max(lua_depth_, 0)));
    const int shown_lua = lua_depth_ < kMaxLuaStack ? lua_depth_ : kMaxLuaStack;
    for (int i = 0; i < shown_lua; ++i) {
        const int index = lua_depth_ - 1 - i;
        lua_recent.push_back(&lua_frames_[index % kMaxLuaStack]);
    }

    std::vector<const JitFnSymbol *> c_frames;
    bool any_spec = false;
    // 只在 GCC 上走 C 栈。TCC 没有展开信息，解释器帧已经写在 lua_frames_ 里。
    // Windows 没有 dladdr/backtrace，抛错函数仍由 FakeluaThrowSpec 带上。
#if !defined(_WIN32)
    if (CurrentJit() == JIT_GCC && !jit_fn_symbols_.empty()) {
        void *pcs[128];
        const int n = backtrace(pcs, 128);
        c_frames.reserve(static_cast<size_t>(std::max(n, 0)));
        for (int i = 0; i < n; ++i) {
            Dl_info info{};
            if (dladdr(pcs[i], &info) == 0 || info.dli_saddr == nullptr) {
                continue;
            }
            const auto it = jit_fn_symbols_.find(info.dli_saddr);
            if (it == jit_fn_symbols_.end()) {
                continue;
            }
            if (it->second.spec) {
                any_spec = true;
            }
            c_frames.push_back(&it->second);
        }
    }
#endif
    const bool hinted = spec_throw_hint_name_ != nullptr && spec_throw_hint_name_[0] != '\0';
    if (!any_spec && !hinted && c_frames.empty()) {
        for (const LuaFrame *fr: lua_recent) {
            out.push_back(MergedLuaFrame{fr->file, fr->line, fr->func, false});
        }
        return;
    }

    if (hinted) {
        out.push_back(MergedLuaFrame{spec_throw_hint_file_, spec_throw_hint_line_, spec_throw_hint_name_, true});
    }
    bool skipped_hint_body = !hinted;
    size_t lua_i = 0;
    for (const JitFnSymbol *sym: c_frames) {
        if (!skipped_hint_body && sym->lua_name == spec_throw_hint_name_) {
            skipped_hint_body = true;
            continue;
        }
        if (lua_i < lua_recent.size() && lua_recent[lua_i]->func != nullptr && sym->lua_name == lua_recent[lua_i]->func) {
            const LuaFrame *fr = lua_recent[lua_i++];
            out.push_back(MergedLuaFrame{fr->file, fr->line, fr->func, false});
            continue;
        }
        if (!sym->spec && !out.empty() && out.back().func != nullptr && sym->lua_name == out.back().func) {
            continue;
        }
        out.push_back(MergedLuaFrame{sym->file.c_str(), sym->line, sym->lua_name.c_str(), sym->spec});
    }
    for (; lua_i < lua_recent.size(); ++lua_i) {
        const LuaFrame *fr = lua_recent[lua_i];
        out.push_back(MergedLuaFrame{fr->file, fr->line, fr->func, false});
    }
}

std::string State::FormatLuaStack() const {
    std::vector<MergedLuaFrame> frames;
    CollectMergedLuaFrames(frames);
    if (frames.empty()) {
        return {};
    }
    const int depth = static_cast<int>(frames.size());
    const int shown = depth < kMaxLuaStack ? depth : kMaxLuaStack;
    const int hidden = depth - shown;
    std::string out = "stack traceback:\n";
    out.reserve(static_cast<size_t>(shown) * 64);
    for (int i = 0; i < shown; ++i) {
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
    if (hidden > 0) {
        out += "\t... (";
        out += std::to_string(hidden);
        out += " more frames)\n";
    }
    return out;
}

std::string State::SpecThrowLoc() const {
    std::vector<MergedLuaFrame> frames;
    CollectMergedLuaFrames(frames);
    if (frames.empty() || frames[0].file == nullptr || frames[0].file[0] == '\0' || frames[0].line <= 0) {
        return {};
    }
    return std::string(frames[0].file) + ":" + std::to_string(frames[0].line);
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
