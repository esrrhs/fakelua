#include "native/mysql/mysql_connection_pool.h"

#include <chrono>
#include <vector>

namespace fakelua::mysql {

// Time helper
static int64_t NowMs() {
    auto tp = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

MysqlConnectionPool::MysqlConnectionPool(const PoolConfig &config, ::fakelua::State *state) : config_(config), state_(state) {
}

MysqlConnectionPool::~MysqlConnectionPool() {
    Close();
    Reap();
    if (!pool_.empty()) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &entry: pool_) {
            // tick 栈上仍在用这条连接时不能 unique_ptr 析构（所有平台，防重入）。
            if (entry.conn && entry.conn->TickDepth() > 0) {
                entry.conn.release();
            }
        }
        pool_.clear();
    }
}

void MysqlConnectionPool::Initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    closed_ = false;
    pool_.clear();
    for (int i = 0; i < config_.pool_size; ++i) {
        PoolEntry entry;
        entry.conn = std::make_unique<MysqlConnection>(state_);
        entry.healthy = false;// not yet connected
        entry.in_use = false;
        entry.last_heartbeat = 0;
        entry.retry_count = 0;

        // Start async connect
        if (entry.conn) {
            entry.conn->Connect(config_.host, config_.port, config_.user, config_.password, config_.database, config_.connect_timeout_ms, config_.ssl, config_.ssl_ca);
        }
        pool_.push_back(std::move(entry));
    }
}

MysqlConnection *MysqlConnectionPool::Acquire() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) return nullptr;

    // Try to find a healthy, connected, non-in-use connection (round-robin)
    size_t start = round_robin_;
    for (size_t i = 0; i < pool_.size(); ++i) {
        size_t idx = (start + i) % pool_.size();
        auto &entry = pool_[idx];
        if (!entry.in_use && entry.healthy && entry.conn && entry.conn->Connected()) {
            entry.in_use = true;
            round_robin_ = (idx + 1) % pool_.size();
            return entry.conn.get();
        }
    }

    return nullptr;// no available connection (caller should tick and retry)
}

void MysqlConnectionPool::Release(MysqlConnection *conn) {
    if (!conn) return;
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &entry: pool_) {
        if (entry.conn.get() == conn) {
            entry.in_use = false;
            break;
        }
    }
}

void MysqlConnectionPool::Tick() {
    if (closed_) {
        Reap();
        return;
    }

    std::vector<MysqlConnection *> to_tick;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        to_tick.reserve(pool_.size());
        for (auto &entry: pool_) {
            if (entry.conn) to_tick.push_back(entry.conn.get());
        }
    }
    for (auto *conn: to_tick) {
        if (closed_) break;
        conn->Tick();
    }
    if (closed_) {
        Reap();
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) return;
    for (auto &entry: pool_) {
        if (!entry.conn) continue;

        bool was_healthy = entry.healthy;
        entry.healthy = entry.conn->Connected();

        if (entry.healthy) {
            if (!was_healthy) {
                entry.last_heartbeat = NowMs();
            }
            if (entry.retry_count > 0) {
                entry.retry_count = 0;
            }
            if (!entry.in_use && config_.heartbeat_interval_ms > 0) {
                int64_t elapsed = NowMs() - entry.last_heartbeat;
                if (elapsed >= config_.heartbeat_interval_ms) {
                    SendHeartbeat(entry);
                }
            }
        } else if (!entry.in_use) {
            if (entry.conn->Connecting()) {
                continue;// handshake in progress, don't tear down
            }
            if (was_healthy) {
                entry.retry_count = 0;
            }
            TryReconnect(entry);
        }
    }
}

void MysqlConnectionPool::Close() {
    std::lock_guard<std::mutex> lock(mutex_);
    closed_ = true;
    bool busy = false;
    for (auto &entry: pool_) {
        if (!entry.conn) continue;
        if (entry.conn->TickDepth() > 0) {
            entry.conn->RequestClose();
            busy = true;
        } else {
            entry.conn->Close();
            entry.conn.reset();
        }
    }
    if (!busy) {
        pool_.clear();
    }
}

void MysqlConnectionPool::Reap() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!closed_) return;
    for (auto &entry: pool_) {
        if (entry.conn && entry.conn->TickDepth() > 0) return;
    }
    for (auto &entry: pool_) {
        if (entry.conn) {
            entry.conn->Close();
            entry.conn.reset();
        }
    }
    pool_.clear();
}

size_t MysqlConnectionPool::TotalCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pool_.size();
}

size_t MysqlConnectionPool::HealthyCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for (const auto &entry: pool_) {
        if (entry.healthy && entry.conn && entry.conn->Connected()) ++count;
    }
    return count;
}

// Private helpers
void MysqlConnectionPool::SendHeartbeat(PoolEntry &entry) {
    if (!entry.conn || !entry.healthy) return;
    if (entry.conn->Ping()) {
        entry.last_heartbeat = NowMs();
    }
}

void MysqlConnectionPool::TryReconnect(PoolEntry &entry) {
    if (!entry.conn || entry.in_use) return;

    if (entry.retry_count > 0) {
        int shift = entry.retry_count - 1;
        if (shift > 20) shift = 20;
        int64_t backoff_ms = static_cast<int64_t>(config_.retry_base_ms) * (1LL << shift);
        int64_t elapsed = NowMs() - entry.last_heartbeat;
        if (elapsed < backoff_ms) return;
    }

    if (entry.retry_count >= config_.max_retries) {
        entry.healthy = false;
        return;
    }

    ++entry.retry_count;

    entry.conn->Close();
    entry.conn->Connect(config_.host, config_.port, config_.user, config_.password, config_.database, config_.connect_timeout_ms, config_.ssl, config_.ssl_ca);
    entry.last_heartbeat = NowMs();
}

}// namespace fakelua::mysql
