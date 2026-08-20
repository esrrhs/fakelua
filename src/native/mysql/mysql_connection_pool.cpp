#include "native/mysql/mysql_connection_pool.h"

#include <chrono>

namespace fakelua::mysql {

// ── Time helper ──

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
        entry.conn = std::make_unique<MysqlConnection>();
        entry.healthy = false;      // not yet connected
        entry.in_use = false;
        entry.last_heartbeat = 0;
        entry.retry_count = 0;

        // Start async connect
        if (entry.conn) {
            entry.conn->connect(config_.host, config_.port, config_.user,
                               config_.password, config_.database);
        }
        pool_.push_back(std::move(entry));
    }
}

MysqlConnection* MysqlConnectionPool::acquire() {
    std::lock_guard<std::mutex> lock(mutex_);

    // Try to find a healthy, connected, non-in-use connection (round-robin)
    size_t start = round_robin_;
    for (size_t i = 0; i < pool_.size(); ++i) {
        size_t idx = (start + i) % pool_.size();
        auto &entry = pool_[idx];
        if (!entry.in_use && entry.healthy && entry.conn && entry.conn->connected()) {
            entry.in_use = true;
            round_robin_ = (idx + 1) % pool_.size();
            return entry.conn.get();
        }
    }

    return nullptr;  // no available connection (caller should tick and retry)
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
        if (!entry.conn) continue;

        // Always tick to drive async IO (connect, handshake, queries)
        entry.conn->tick();

        // Update health based on actual connection state
        bool was_healthy = entry.healthy;
        entry.healthy = entry.conn->connected();

        if (entry.healthy) {
            // Reset retry count on successful connection
            if (entry.retry_count > 0) {
                entry.retry_count = 0;
            }

            // Heartbeat check (only for connections not in use)
            if (!entry.in_use && config_.heartbeat_interval_ms > 0) {
                int64_t elapsed = now_ms() - entry.last_heartbeat;
                if (elapsed >= config_.heartbeat_interval_ms) {
                    send_heartbeat(entry);
                }
            }
        } else if (was_healthy) {
            // Connection just lost — initiate reconnect
            entry.retry_count = 0;  // reset for fresh retry cycle
            try_reconnect(entry);
        } else if (!entry.in_use) {
            // Not connected and not in use — try reconnect with backoff
            try_reconnect(entry);
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
        if (entry.healthy && entry.conn && entry.conn->connected()) ++count;
    }
    return count;
}

// ── Private helpers ──

void MysqlConnectionPool::send_heartbeat(PoolEntry &entry) {
    if (!entry.conn || !entry.healthy) return;

    // Send COM_PING to keep connection alive
    std::string payload(1, static_cast<char>(0x0E));  // COM_PING = 0x0E
    entry.conn->send_packet(0, payload.data(), payload.size());

    // Update heartbeat time (optimistic; will be reset if ping fails)
    entry.last_heartbeat = now_ms();
}

void MysqlConnectionPool::try_reconnect(PoolEntry &entry) {
    if (!entry.conn) return;

    // Exponential backoff
    if (entry.retry_count > 0) {
        int64_t backoff_ms = config_.retry_base_ms * (1 << (entry.retry_count - 1));
        int64_t elapsed = now_ms() - entry.last_heartbeat;
        if (elapsed < backoff_ms) return;  // wait more
    }

    if (entry.retry_count >= config_.max_retries) {
        entry.healthy = false;
        return;  // exceeded max retries
    }

    ++entry.retry_count;

    // Attempt reconnect
    entry.conn->close();
    entry.conn->connect(config_.host, config_.port, config_.user, config_.password, config_.database);
    entry.last_heartbeat = now_ms();
}

}  // namespace fakelua::mysql
