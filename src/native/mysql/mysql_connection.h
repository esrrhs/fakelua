#pragma once

// mysql_connection.h — async MySQL client using libmysqlclient (MariaDB Connector/C)
// MYSQL_OPT_NONBLOCK + libevent wait on mysql_get_socket().

#include "fakelua.h"
#include "native/native_io_context.h"

#ifdef __cplusplus
#ifndef HAVE_BOOL
#define HAVE_BOOL 1
#endif
#endif
#include <mysql.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct event;

namespace fakelua {
struct CVar;
class State;
class NativeObject;
}// namespace fakelua

// FieldCell 和 FieldCellToCVar 声明在独立的轻量头文件中（不依赖 mysql.h/libevent），
// 可直接在测试代码里 include。
#include "native/mysql/mysql_field_convert.h"

namespace fakelua::mysql {

struct StmtParam {
    bool is_null = false;
    std::string value;
};


// C++ → Lua 异步回调：全局函数名 + 绑定参数。
// fakelua 没有 GC，运行期闭包活在临时 arena 上，顶层 Call 的 State::Reset() 会回收，
// 无法安全地跨 tick 持有闭包裸指针。因此异步回调只接受【全局函数名】；脚本需要上下文时
// 用绑定参数（mysql.connect / conn:query 等名字后面的多余实参）：
// 登记当下把纯数据参数序列化成自有的字节串 bound（nil/bool/number/string/table），
// 跨任意次 Reset 暂存；结果派发时在当前 tick 帧内 decode 到临时 arena，追加在固定参数
// （conn, err, result, ...）之后调用函数。bound 随 query 完成/连接销毁释放，
// 内存只与在途 query 数相关，不占用 const arena。
struct ResultCallback {
    std::string name;
    std::string bound;// wire 编码的绑定参数元组；空串表示无绑定参数
    // 登记回调时所在的脚本引擎。结果回来后派回【同一引擎】：在哪个引擎发起的 IO，
    // 回调就执行哪个引擎的编译产物，与内联闭包自带 func_ptr 的语义一致。
    JITType jit = JIT_TCC;

    [[nodiscard]] bool Empty() const { return name.empty(); }
};

enum class SslMode {
    Disable = 0,
    Enable,
    Require,
};

enum class MysqlErrorType {
    None = 0,
    Connection,
    Authentication,
    Syntax,
    Timeout,
    Protocol,
    Server,
    Unknown
};

struct MysqlError {
    MysqlErrorType type = MysqlErrorType::None;
    int code = 0;
    std::string message;
    std::string sql_state;
};

struct ResultsetData {

    bool is_resultset = false;
    std::vector<std::pair<std::string, int>> columns;
    std::vector<std::vector<FieldCell>> rows;
    uint64_t affected_rows = 0;
    uint64_t last_insert_id = 0;
    std::string info;
};

class MysqlConnection {
public:
    explicit MysqlConnection(::fakelua::State *state);
    ~MysqlConnection();

    MysqlConnection(const MysqlConnection &) = delete;
    MysqlConnection &operator=(const MysqlConnection &) = delete;

    void Connect(const std::string &host, uint16_t port, const std::string &user, const std::string &password, const std::string &database, int timeout_ms = 0, SslMode ssl = SslMode::Disable, std::string ssl_ca = {});

    void Query(const std::string &sql);
    void StmtPrepare(const std::string &sql);
    void StmtExecute(uint32_t stmt_id, const std::vector<StmtParam> &params);
    void StmtClose(uint32_t stmt_id);
    bool Ping();
    void Close();
    void Tick();

    MysqlError LastError() const;
    static bool IsRetryable(MysqlErrorType type);

    void SetConnectCallback(const ResultCallback &cb);
    void SetResultCallback(const ResultCallback &cb);
    void SetState(::fakelua::State *state);
    void SetNativeObject(::fakelua::NativeObject *obj);

    bool Connected() const;
    bool Connecting() const;

    // 单测：跳过握手，把连接标成可被池 Acquire。生产路径不会调用。
    void MarkConnectedForTest();

    int TickDepth() const;
    bool ClosePending() const;
    void RequestClose();

private:
    native::IoContext &io_;
    MYSQL *mysql_ = nullptr;
    event *wait_ev_ = nullptr;

    enum class ConnState { Idle, Connecting, Handshaking, Ready, Querying, Error };
    ConnState state_ = ConnState::Idle;
    ::fakelua::State *lua_state_ = nullptr;
    ::fakelua::NativeObject *native_obj_ = nullptr;

    ResultCallback connect_cb_;
    ResultCallback result_cb_;
    std::string last_sql_;

    // 同连接飞行中再次发起的 query：排队而不是报 "connection not ready"，
    // 由 Tick() 在上一条结果派发完毕、连接回到 Ready 后依次启动。
    struct QueuedQuery {
        std::string sql;
        ResultCallback cb;
    };
    std::vector<QueuedQuery> queued_queries_;

    enum class QueryType { None, Query, StmtPrepare, StmtExecute, Ping };
    QueryType query_type_ = QueryType::None;

    enum class WaitOp { None, Connect, Query, Store, Next, Ping, StmtPrepare, StmtExecute, StmtStore };
    WaitOp wait_op_ = WaitOp::None;

    std::unordered_map<uint32_t, MYSQL_STMT *> prepared_statements_;
    uint32_t next_stmt_id_ = 1;
    uint32_t pending_exec_stmt_id_ = 0;
    MYSQL_STMT *pending_stmt_ = nullptr;
    std::vector<StmtParam> pending_stmt_params_;
    std::vector<MYSQL_BIND> pending_binds_;
    std::vector<unsigned long> pending_bind_lens_;

    MysqlError last_error_;

    std::string host_;
    std::string user_;
    std::string password_;
    std::string database_;
    uint16_t port_ = 3306;
    int timeout_ms_ = 0;
    SslMode ssl_mode_ = SslMode::Disable;
    std::string ssl_ca_;

    int64_t connect_start_ms_ = 0;

    bool pending_connect_ = false;
    std::string pending_connect_err_;
    bool pending_result_ = false;
    std::string pending_result_err_;
    std::vector<ResultsetData> pending_results_;
    bool has_pending_stmt_id_ = false;
    uint32_t pending_stmt_id_ = 0;
    uint32_t dispatch_stmt_id_ = 0;

    bool ready_ = false;
    int tick_depth_ = 0;
    bool close_pending_ = false;

    native::LifeToken life_;

    void DispatchConnect(const char *err_msg);
    void DispatchResult(const char *err_msg);
    void DispatchCallbackWithResult(const ResultCallback &cb, const char *err_msg);
    void SetError(MysqlErrorType type, uint16_t code, const std::string &msg, const std::string &sql_state);

    // 统一的回调调用入口：按函数名查 VM 注册表并派发。
    // 调用方需已把固定参数和 decode 后的绑定参数拼进 args。
    // 回调缺失或不可调用时返回空 CVar（调用方无需关心返回值）。
    void InvokeCallback(const ResultCallback &cb, CVar *args, int n);
    // 连接可用时启动下一条排队 query；连接进入终态（Error/Idle）时
    // 逐条给排队的 query 回调派发错误，保证"每条 query 恰好一次回调"。
    void DrainQueryQueue();

    void Teardown();
    void ApplySsl();
    void ArmWait(int status);
    void ClearWait();
    void Continue(int ready);
    void FinishConnect(MYSQL *ret);
    void StartStore();
    void FinishQueryOk();
    ResultsetData ConsumeResult(MYSQL_RES *res, bool is_resultset);
    ResultsetData ConsumeStmtResult(MYSQL_STMT *stmt);

    static CVar ResultsetToLua(::fakelua::State *s, const ResultsetData &result);
};

}// namespace fakelua::mysql
