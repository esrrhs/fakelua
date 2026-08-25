#include "native/mysql/mysql_connection_pool.h"

#include <chrono>
#include <vector>

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
    reap();
    if (!pool_.empty()) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &entry : pool_) {
            // tick 栈上仍在用这条连接时不能 unique_ptr 析构，宁可泄漏也不能 UAF
            if (entry.conn && entry.conn->tick_depth() > 0) {
                entry.conn.release();
            }
        }
        pool_.clear();
    }
}

void MysqlConnectionPool::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    closed_ = false;
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
                               config_.password, config_.database, config_.connect_timeout_ms);
        }
        pool_.push_back(std::move(entry));
    }
}

MysqlConnection* MysqlConnectionPool::acquire() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) return nullptr;

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
    if (closed_) {
        reap();
        return;
    }

    std::vector<MysqlConnection *> to_tick;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        to_tick.reserve(pool_.size());
        for (auto &entry : pool_) {
            if (entry.conn) to_tick.push_back(entry.conn.get());
        }
    }
    for (auto *conn : to_tick) {
        if (closed_) break;
        conn->tick();
    }
    if (closed_) {
        reap();
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) return;
    for (auto &entry : pool_) {
        if (!entry.conn) continue;

        bool was_healthy = entry.healthy;
        entry.healthy = entry.conn->connected();

        if (entry.healthy) {
            if (!was_healthy) {
                entry.last_heartbeat = now_ms();
            }
            if (entry.retry_count > 0) {
                entry.retry_count = 0;
            }
            if (!entry.in_use && config_.heartbeat_interval_ms > 0) {
                int64_t elapsed = now_ms() - entry.last_heartbeat;
                if (elapsed >= config_.heartbeat_interval_ms) {
                    send_heartbeat(entry);
                }
            }
        } else if (!entry.in_use) {
            if (entry.conn->connecting()) {
                continue; // handshake in progress, don't tear down
            }
            if (was_healthy) {
                entry.retry_count = 0;
            }
            try_reconnect(entry);
        }
    }
}

void MysqlConnectionPool::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    closed_ = true;
    bool busy = false;
    for (auto &entry : pool_) {
        if (!entry.conn) continue;
        if (entry.conn->tick_depth() > 0) {
            entry.conn->request_close();
            busy = true;
        } else {
            entry.conn->close();
            entry.conn.reset();
        }
    }
    if (!busy) {
        pool_.clear();
    }
}

void MysqlConnectionPool::reap() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!closed_) return;
    for (auto &entry : pool_) {
        if (entry.conn && entry.conn->tick_depth() > 0) return;
    }
    for (auto &entry : pool_) {
        if (entry.conn) {
            entry.conn->close();
            entry.conn.reset();
        }
    }
    pool_.clear();
}

size_t MysqlConnectionPool::total_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pool_.size();
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
    if (entry.conn->ping()) {
        entry.last_heartbeat = now_ms();
    }
}

void MysqlConnectionPool::try_reconnect(PoolEntry &entry) {
    if (!entry.conn || entry.in_use) return;

    if (entry.retry_count > 0) {
        int shift = entry.retry_count - 1;
        if (shift > 20) shift = 20;
        int64_t backoff_ms = static_cast<int64_t>(config_.retry_base_ms) * (1LL << shift);
        int64_t elapsed = now_ms() - entry.last_heartbeat;
        if (elapsed < backoff_ms) return;
    }

    if (entry.retry_count >= config_.max_retries) {
        entry.healthy = false;
        return;
    }

    ++entry.retry_count;

    entry.conn->close();
    entry.conn->connect(config_.host, config_.port, config_.user, config_.password, config_.database,
                        config_.connect_timeout_ms);
    entry.last_heartbeat = now_ms();
}

}  // namespace fakelua::mysql
