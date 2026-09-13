#include "state/state.h"
#include "fakelua.h"
#include "jit/tcc_jit.h"
#include "native/compress/native_compress.h"
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
#include "native/container/native_container.h"
#include "native/redis/native_redis.h"
#include "native/runtime/native_runtime.h"
#include "native/serialize/native_serialize.h"
#include "native/sqlite/native_sqlite.h"
#include "native/timer/native_timer.h"
#include "native/toml/native_toml.h"
#include "native/utf8/native_utf8.h"
#include "native/url/native_url.h"
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

void State::SetLogFile(const std::string &path, size_t max_size, size_t max_files) {
    log_sink_.reset();
    if (!path.empty()) {
        log_sink_ = decltype(log_sink_)(CreateLogSink(path, max_size, max_files), &DestroyLogSink);
    }
}

State::State(const StateConfig &config) : config_(config), compiler_(this), const_string_(this) {
    utf8_io::Init();
    log_level_ = static_cast<LogLevel>(config_.log_level);
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
