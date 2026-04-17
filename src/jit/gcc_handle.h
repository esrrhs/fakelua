#pragma once

#include "jit/jit_common.h"
#include <string>

namespace fakelua {

class GCCHandle : public JITHandle {
public:
    GCCHandle(std::string c_file, std::string so_file, void *dl_handle)
        : c_file_(std::move(c_file)), so_file_(std::move(so_file)), dl_handle_(dl_handle) {
    }

    ~GCCHandle() override;

    [[nodiscard]] const std::string &GetCFile() const {
        return c_file_;
    }

    [[nodiscard]] const std::string &GetSoFile() const {
        return so_file_;
    }

    [[nodiscard]] void *GetDlHandle() const {
        return dl_handle_;
    }

private:
    std::string c_file_;
    std::string so_file_;
    void *dl_handle_ = nullptr;
};

typedef std::shared_ptr<GCCHandle> GCCHandlePtr;

}// namespace fakelua
