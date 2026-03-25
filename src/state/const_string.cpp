#include "const_string.h"
#include "state.h"
#include "util/common.h"
#include "var/var_string.h"

namespace fakelua {

ConstString::ConstString(State *s) : s_(s) {
}

int64_t ConstString::Alloc(const std::string_view &str) {
    if (const auto it = str_to_id_map_.find(str); it != str_to_id_map_.end()) {
        return it->second;
    }

    auto ptr = static_cast<VarString *>(s_->GetHeap().GetConstAllocator().Alloc(sizeof(VarString) + str.size()));
    new (ptr) VarString(str);
    auto id = reinterpret_cast<int64_t>(ptr);

    auto key = ptr->Str();
    str_to_id_map_.emplace(key, id);
    return id;
}

std::string_view ConstString::GetString(int64_t id) {
    const auto ptr = reinterpret_cast<VarString *>(id);
    return ptr->Str();
}

VarString *ConstString::GetVarString(int64_t id) {
    return reinterpret_cast<VarString *>(id);
}

}// namespace fakelua
