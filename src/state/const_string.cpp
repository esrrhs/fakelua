#include "const_string.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

int64_t ConstString::Alloc(const std::string_view &str) {
    if (const auto it = str_to_id_map_.find(str); it != str_to_id_map_.end()) {
        return it->second;
    }

    const auto id = static_cast<int64_t>(str_to_id_map_.size()) + 1;
    auto save_str = std::to_string(id);
    str_to_id_map_.emplace(save_str, id);
    id_to_str_map_.emplace(id, std::string(str));
    return id;
}

std::string_view ConstString::Get(int64_t id) const {
    const auto it = id_to_str_map_.find(id);
    if (it == id_to_str_map_.end()) {
        return std::string_view();
    }
    return it->second;
}

}// namespace fakelua
