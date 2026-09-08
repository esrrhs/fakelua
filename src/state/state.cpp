#include "state/state.h"
#include "fakelua.h"
#include "jit/tcc_jit.h"
#include "native/os/native_os.h"
#include "native/utf8/native_utf8.h"
#include "native/io/native_io.h"
#include "native/net/native_net.h"
#include "native/timer/native_timer.h"
#include "native/serialize/native_serialize.h"
#include "native/protobuf/native_protobuf.h"
#include "native/crypto/native_crypto.h"
#include "native/mysql/native_mysql.h"
#include "native/compress/native_compress.h"
#include "native/json/native_json.h"
#include "native/csv/native_csv.h"
#include "native/sqlite/native_sqlite.h"
#include "native/event/native_event.h"
#include "native/random/native_random.h"
#include "native/yaml/native_yaml.h"
#include "native/xml/native_xml.h"
#include "native/toml/native_toml.h"
#include "native/ini/native_ini.h"
#include "native/log/native_log.h"
#include "native/native_io_context.h"

namespace fakelua {

// 在这里而不是头文件里：io_context_ 用的是不完整类型。
State::~State() = default;

native::IoContext &State::GetIoContext() {
    if (!io_context_) {
        io_context_ = std::make_unique<native::IoContext>();
    }
    return *io_context_;
}

State::State(const StateConfig &config) : config_(config), compiler_(this), const_string_(this) {
    RegisterNativeObjectApi(this);
    net::RegisterNetLibraryApi(this);
    timer::RegisterTimerLibraryApi(this);
    serialize::RegisterSerializeLibraryApi(this);
    protobuf::RegisterProtobufLibraryApi(this);
    crypto::RegisterCryptoLibraryApi(this);
    compress::RegisterCompressLibraryApi(this);
    json::RegisterJsonLibraryApi(this);
    csv::RegisterCsvLibraryApi(this);
    sqlite::RegisterSqliteLibraryApi(this);
    mysql::RegisterMysqlLibraryApi(this);
    mysql::RegisterMysqlPoolApi(this);
    event::RegisterEventLibraryApi(this);
    random::RegisterRandomLibraryApi(this);
    yaml::RegisterYamlLibraryApi(this);
    xml::RegisterXmlLibraryApi(this);
    toml::RegisterTomlLibraryApi(this);
    ini::RegisterIniLibraryApi(this);
    log::RegisterLogLibraryApi(this);
}

}// namespace fakelua
