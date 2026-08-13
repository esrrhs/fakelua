#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

// 为了性能考虑，字符串只是字符串，并不能自动转成数字参与计算。
// 字符串也分为了常量字符串和临时字符串，常量字符串是在编译期确定的，会自动分配唯一的编号，方便快速查找。
// VarString是临时字符串，运行期间生成，直接从池子里切出来，最后一口气释放。
class VarString {
public:
    explicit VarString(const std::string_view &str) {
        DEBUG_ASSERT(str.size() <= static_cast<size_t>(std::numeric_limits<int>::max()));
        size_ = static_cast<int>(str.size());
        hash_ = 0;
        memcpy(&data_[0], str.data(), size_);
    }

    // 返回字符串视图
    [[nodiscard]] std::string_view Str() const {
        return {data_, static_cast<std::string_view::size_type>(size_)};
    }

    [[nodiscard]] size_t Size() const {
        return static_cast<size_t>(size_);
    }

    // djb2。必须与 c_runtime_header.h 中的 FlHashString 保持一致，否则宿主与 JIT
    // 生成的代码会把同一个字符串键算到不同的桶里。
    [[nodiscard]] static uint32_t HashOf(const std::string_view &str) {
        uint32_t hash_val = 5381;
        for (const char c: str) {
            hash_val = ((hash_val << 5) + hash_val) + static_cast<uint8_t>(c);
        }
        return (hash_val == 0) ? 1 : hash_val;
    }

    [[nodiscard]] uint32_t Hash() const {
        if (hash_ == 0) {
            hash_ = HashOf(Str());
        }
        return hash_;
    }

    static VarString *AllocTemp(State *state, const std::string_view &str);

private:
    int size_ = 0;
    mutable uint32_t hash_ = 0;
    char data_[0];
};

static_assert(sizeof(VarString) == 8, "VarString size must be 8 bytes");

}// namespace fakelua
