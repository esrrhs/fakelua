#pragma once

// mysql_connection_pool.h — connection pool for MySQL.
// Manages a set of reusable connections with health checking,
// auto-reconnect, and heartbeat keepalive.

#include "native/mysql/mysql_connection.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace fakelua::mysql {

struct PoolConfig {
    std::string host = "127.0.0.1";
    uint16_t port = 3306;
    std::string user;
    std::string password;
    std::string database;
    int pool_size = 4;                // number of connections in pool
    int connect_timeout_ms = 5000;    // TCP connect timeout
    int read_timeout_ms = 5000;       // read timeout
    int heartbeat_interval_ms = 30000;// heartbeat interval (0 = disabled)
    int max_retries = 3;              // max auto-reconnect retries
    int retry_base_ms = 1000;         // exponential backoff base
};

class MysqlConnectionPool {
public:
    // state 只用来取事件循环，池里的连接全都跑在它上面。
    MysqlConnectionPool(const PoolConfig &config, ::fakelua::State *state);
    ~MysqlConnectionPool();

    MysqlConnectionPool(const MysqlConnectionPool &) = delete;
    MysqlConnectionPool &operator=(const MysqlConnectionPool &) = delete;

    // Initialize the pool: create connections
    void Initialize();

    // Get a connection from the pool (round-robin)
    // Returns nullptr if no healthy connection available
    MysqlConnection *Acquire();

    // Release a connection back to the pool
    void Release(MysqlConnection *conn);

    // Tick all connections (heartbeat, reconnect)
    void Tick();

    // Close all connections. If a connection is inside tick()/Lua callback,
    // unique_ptrs are kept until reap() (same deferred-close idea as net).
    void Close();

    // Destroy idle connections after a deferred close(). Safe to call anytime.
    void Reap();

    bool Closed() const {
        return closed_;
    }

    // Stats
    size_t TotalCount() const;
    size_t HealthyCount() const;

private:
    PoolConfig config_;
    ::fakelua::State *state_ = nullptr;
    bool closed_ = false;

    struct PoolEntry {
        std::unique_ptr<MysqlConnection> conn;
        bool in_use = false;
        bool healthy = false;
        int64_t last_heartbeat = 0;// last successful heartbeat time
        int retry_count = 0;       // current retry count
    };

    std::vector<PoolEntry> pool_;
    size_t round_robin_ = 0;// round-robin index
    mutable std::mutex mutex_;

    // Create a new connection
    std::unique_ptr<MysqlConnection> CreateConnection();

    // Check if a connection is healthy
    bool CheckHealthy(PoolEntry &entry);

    // Send heartbeat (COM_PING)
    void SendHeartbeat(PoolEntry &entry);

    // Auto-reconnect a connection
    void TryReconnect(PoolEntry &entry);
};

}// namespace fakelua::mysql
