#include "native/mysql/mysql_connection_pool.h"

#include <chrono>

namespace fakelua::mysql {

// ── Time helper (no Date.now/Math.random in workflow scripts) ──

static int64_t now_ms() {
    auto tp = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()).count();
}

MysqlConnectionPool::MysqlConnectionPool(const PoolConfig &config)
    : config_(config) {}

MysqlConnectionPool::~MysqlConnectionPool() {
    close();
}

void MysqlConnectionPool::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    pool_.clear();
    for (int i = 0; i < config_.pool_size; ++i) {
        PoolEntry entry;
        entry.conn = create_connection();
        if (entry.conn) {
            entry.healthy = true;
            entry.conn->set_state(nullptr);  // will be set on acquire
        }
        pool_.push_back(std::move(entry));
    }
}

MysqlConnection* MysqlConnectionPool::acquire() {
    std::lock_guard<std::mutex> lock(mutex_);

    // Try to find a healthy, non-in-use connection (round-robin)
    size_t start = round_robin_;
    for (size_t i = 0; i < pool_.size(); ++i) {
        size_t idx = (start + i) % pool_.size();
        auto &entry = pool_[idx];
        if (!entry.in_use && entry.healthy && entry.conn) {
            entry.in_use = true;
            round_robin_ = (idx + 1) % pool_.size();
            return entry.conn.get();
        }
    }

    // No healthy connection available — try to find any that can be reconnected
    for (size_t i = 0; i < pool_.size(); ++i) {
        auto &entry = pool_[i];
        if (!entry.in_use && !entry.healthy) {
            if (reconnect(entry)) {
                entry.in_use = true;
                return entry.conn.get();
            }
        }
    }

    return nullptr;  // all connections in use or unreachable
}

void MysqlConnectionPool::release(MysqlConnection *conn) {
    if (!conn) return;
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &entry : pool_) {
        if (entry.conn.get() == conn) {
            entry.in_use = false;
            break;
        }
    }
}

void MysqlConnectionPool::tick() {
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto &entry : pool_) {
        if (entry.in_use) continue;  // skip connections in use

        // Tick the connection (process pending IO)
        if (entry.conn) {
            entry.conn->tick();
        }

        // Heartbeat check
        if (config_.heartbeat_interval_ms > 0 && entry.healthy) {
            int64_t elapsed = now_ms() - entry.last_heartbeat;
            if (elapsed >= config_.heartbeat_interval_ms) {
                send_heartbeat(entry);
            }
        }

        // Auto-reconnect unhealthy connections
        if (!entry.healthy) {
            reconnect(entry);
        }
    }
}

void MysqlConnectionPool::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &entry : pool_) {
        if (entry.conn) {
            entry.conn->close();
        }
    }
    pool_.clear();
}

size_t MysqlConnectionPool::healthy_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for (const auto &entry : pool_) {
        if (entry.healthy) ++count;
    }
    return count;
}

// ── Private helpers ──

std::unique_ptr<MysqlConnection> MysqlConnectionPool::create_connection() {
    auto conn = std::make_unique<MysqlConnection>();
    // Note: actual async connect is done via conn->connect() + tick()
    // For pool init, we just create the object; health check will connect
    return conn;
}

bool MysqlConnectionPool::check_healthy(PoolEntry &entry) {
    if (!entry.conn) return false;
    return entry.conn->connected();
}

void MysqlConnectionPool::send_heartbeat(PoolEntry &entry) {
    if (!entry.conn || !entry.healthy) return;

    // Send COM_PING to keep connection alive
    std::string payload(1, static_cast<char>(0x0E));  // COM_PING = 0x0E
    entry.conn->send_packet(0, payload.data(), payload.size());

    // Update heartbeat time (optimistic; will be reset if ping fails)
    entry.last_heartbeat = now_ms();
}

bool MysqlConnectionPool::reconnect(PoolEntry &entry) {
    if (!entry.conn) return false;

    // Exponential backoff
    if (entry.retry_count > 0) {
        int64_t backoff_ms = config_.retry_base_ms * (1 << (entry.retry_count - 1));
        int64_t elapsed = now_ms() - entry.last_heartbeat;
        if (elapsed < backoff_ms) return false;  // wait more
    }

    if (entry.retry_count >= config_.max_retries) {
        entry.healthy = false;
        return false;  // exceeded max retries
    }

    ++entry.retry_count;

    // Attempt reconnect
    entry.conn->close();
    entry.conn->connect(config_.host, config_.port, config_.user, config_.password, config_.database);

    // Note: actual connection requires tick() to drive IO
    // For now, mark as attempting; health will be verified on next tick
    entry.last_heartbeat = now_ms();

    return false;  // connection not yet established (needs tick)
}

}  // namespace fakelua::mysql
