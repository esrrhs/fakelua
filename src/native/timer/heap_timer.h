#pragma once

#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

// 4-ary min-heap timer (header-only).
// Source: https://github.com/esrrhs/heap_timer (MIT)
// Integrated into fakelua as a standalone timer library.

namespace fakelua::timer {

class HeapTimer {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using TimerId = uint64_t;

    HeapTimer() = default;
    ~HeapTimer() = default;

    HeapTimer(const HeapTimer&) = delete;
    HeapTimer& operator=(const HeapTimer&) = delete;

    HeapTimer(HeapTimer&& other) noexcept
        : timer_id_(other.timer_id_),
          heap_(std::move(other.heap_)),
          heap_size_(other.heap_size_),
          timer_map_(std::move(other.timer_map_)) {
        other.timer_id_ = 0;
        other.heap_size_ = 0;
    }

    HeapTimer& operator=(HeapTimer&& other) noexcept {
        if (this != &other) {
            timer_id_ = other.timer_id_;
            heap_ = std::move(other.heap_);
            heap_size_ = other.heap_size_;
            timer_map_ = std::move(other.timer_map_);
            other.timer_id_ = 0;
            other.heap_size_ = 0;
        }
        return *this;
    }

    // Not thread-safe. delay_ms is relative to a monotonic clock.
    TimerId Add(uint32_t delay_ms) {
        return AddAt(Clock::now() + std::chrono::milliseconds(delay_ms));
    }

    TimerId AddAt(TimePoint when) {
        if (heap_size_ >= static_cast<size_t>(std::numeric_limits<int32_t>::max())) {
            return 0;
        }
        if (heap_size_ >= heap_.size()) {
            size_t n = 16;
            if (n <= heap_.size()) {
                n = heap_.size() * 3 / 2;
            }
            heap_.resize(n);
        }
        auto t = std::make_shared<TimerNode>();
        t->i = static_cast<int32_t>(heap_size_++);
        t->when = when;
        t->id = ++timer_id_;
        heap_[t->i] = t;
        SiftUp(t->i);
        timer_map_[t->id] = t;
        return t->id;
    }

    bool Del(TimerId id) {
        auto it = timer_map_.find(id);
        if (it == timer_map_.end()) {
            return false;
        }
        auto t = it->second;
        auto i = t->i;
        if (i < 0 || static_cast<size_t>(i) >= heap_size_ || heap_[i] != t) {
            return false;
        }

        timer_map_.erase(it);
        t->i = -1;

        heap_size_--;
        if (static_cast<size_t>(i) == heap_size_) {
            heap_[i] = nullptr;
        } else {
            heap_[i] = heap_[heap_size_];
            heap_[heap_size_] = nullptr;
            heap_[i]->i = i;
            SiftUp(i);
            SiftDown(i);
        }

        return true;
    }

    std::vector<TimerId> Update() {
        auto now = Clock::now();
        std::vector<TimerId> ret;
        while (heap_size_ > 0) {
            auto t = heap_[0];
            if (t->when > now) {
                break;
            }

            heap_size_--;
            if (heap_size_ == 0) {
                heap_[0] = nullptr;
            } else {
                heap_[0] = heap_[heap_size_];
                heap_[heap_size_] = nullptr;
                heap_[0]->i = 0;
                SiftDown(0);
            }

            t->i = -1;
            timer_map_.erase(t->id);
            ret.push_back(t->id);
        }

        return ret;
    }

    size_t Size() const {
        return heap_size_;
    }

private:
    void SiftUp(int32_t i) {
        auto when = heap_[i]->when;
        auto tmp = heap_[i];
        while (i > 0) {
            auto p = (i - 1) / 4;
            if (when >= heap_[p]->when) {
                break;
            }
            heap_[i] = heap_[p];
            heap_[i]->i = i;
            heap_[p] = tmp;
            tmp->i = p;
            i = p;
        }
    }

    void SiftDown(int32_t i) {
        auto when = heap_[i]->when;
        auto tmp = heap_[i];
        while (true) {
            auto c = i * 4 + 1;
            auto c3 = c + 2;
            if (c < 0 || static_cast<size_t>(c) >= heap_size_) {
                break;
            }
            auto w = heap_[c]->when;
            if (static_cast<size_t>(c + 1) < heap_size_ && heap_[c + 1]->when < w) {
                w = heap_[c + 1]->when;
                c++;
            }
            if (c3 >= 0 && static_cast<size_t>(c3) < heap_size_) {
                auto w3 = heap_[c3]->when;
                if (static_cast<size_t>(c3 + 1) < heap_size_ && heap_[c3 + 1]->when < w3) {
                    w3 = heap_[c3 + 1]->when;
                    c3++;
                }
                if (w3 < w) {
                    w = w3;
                    c = c3;
                }
            }
            if (w >= when) {
                break;
            }
            heap_[i] = heap_[c];
            heap_[i]->i = i;
            heap_[c] = tmp;
            tmp->i = c;
            i = c;
        }
    }

    struct TimerNode {
        TimePoint when{};
        TimerId id = 0;
        int32_t i = 0;
    };

    using TimerNodePtr = std::shared_ptr<TimerNode>;
    TimerId timer_id_ = 0;
    std::vector<TimerNodePtr> heap_;
    size_t heap_size_ = 0;
    std::unordered_map<TimerId, TimerNodePtr> timer_map_;
};

} // namespace fakelua::timer
